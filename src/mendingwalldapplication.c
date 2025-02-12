/*
 * Copyright (C) 2025 Lawrence Murray <lawrence@indii.org>
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include <config.h>
#include <utility.h>
#include <mendingwalldapplication.h>

#define G_SETTINGS_ENABLE_BACKEND 1
#include <gio/gio.h>
#include <gio/gsettingsbackend.h>
#include <libportal/portal.h>

struct _MendingwallDApplication {
  GApplication parent_instance;

  XdpPortal* portal;
  GSettings* global;
  GPtrArray* theme_settings;
  GPtrArray* theme_monitors;
  GPtrArray* menu_monitors;
};

G_DEFINE_TYPE(MendingwallDApplication, mendingwall_d_application, G_TYPE_APPLICATION)

static void on_changed_setting(GSettings* settings, gchar* key) {
  save_setting(settings, key);
}

static void on_changed_file(GFileMonitor* monitor, GFile* file) {
  g_autoptr(GFile) dir = g_file_new_for_path(get_user_config_dir());
  g_autofree char* path = g_file_get_relative_path(dir, file);
  save_file(path);
}

static void on_changed_app(GFileMonitor* monitor, GFile* file) {
  g_autofree char* basename = g_file_get_basename(file);
  tidy_app(basename);
}

static void watch_themes(MendingwallDApplication* self) {
  /* watch settings */
  g_auto(GStrv) schema_ids = get_themes_schema_ids();
  foreach(schema_id, schema_ids) {
    GSettings* settings = g_settings_new(schema_id);
    g_signal_connect(settings, "changed", G_CALLBACK(on_changed_setting),
        NULL);
    g_ptr_array_add(self->theme_settings, settings);
  }

  /* watch config files */
  g_autoptr(GFile) dir = g_file_new_for_path(get_user_config_dir());
  g_auto(GStrv) paths = get_themes_files();
  foreach(path, paths) {
    g_autoptr(GFile) file = g_file_resolve_relative_path(dir, path);
    GFileMonitor* monitor = g_file_monitor_file(file, G_FILE_MONITOR_NONE,
        NULL, NULL);
    g_signal_connect(monitor, "changed", G_CALLBACK(on_changed_file), NULL);
    g_ptr_array_add(self->theme_monitors, monitor);
  }
}

static void unwatch_themes(MendingwallDApplication* self) {
  g_ptr_array_set_size(self->theme_monitors, 0);
  g_ptr_array_set_size(self->theme_settings, 0);
}

static void watch_menus(MendingwallDApplication* self) {
  /* watch application directories */
  foreach(path, get_system_data_dirs()) {
    g_autoptr(GFile) dir = g_file_new_build_filename(path, "applications", NULL);
    GFileMonitor* monitor = g_file_monitor_directory(dir, G_FILE_MONITOR_NONE,
        NULL, NULL);
    g_signal_connect(monitor, "changed", G_CALLBACK(on_changed_app), self);
    g_ptr_array_add(self->menu_monitors, monitor);
  }
}

static void unwatch_menus(MendingwallDApplication* self) {
  g_ptr_array_set_size(self->menu_monitors, 0);
}

static void on_changed_themes(gpointer user_data) {
  MendingwallDApplication* self = MENDINGWALL_D_APPLICATION(user_data);
  gboolean themes = g_settings_get_boolean(self->global, "themes");
  gboolean menus = g_settings_get_boolean(self->global, "menus");
  if (themes) {
    watch_themes(self);
    save_themes();
  } else {
    unwatch_themes(self);
  }
  if (!themes && !menus) {
    g_application_quit(G_APPLICATION(self));
  }
}

static void on_changed_menus(gpointer user_data) {
  MendingwallDApplication* self = MENDINGWALL_D_APPLICATION(user_data);
  gboolean themes = g_settings_get_boolean(self->global, "themes");
  gboolean menus = g_settings_get_boolean(self->global, "menus");
  if (menus) {
    watch_menus(self);
    tidy_menus();
  } else {
    unwatch_menus(self);
    untidy_menus();
  }
  if (!themes && !menus) {
    g_application_quit(G_APPLICATION(self));
  }
}

static void on_session_state_changed(MendingwallDApplication* self, gboolean,
  XdpLoginSessionState* state) {
  if (*state == XDP_LOGIN_SESSION_QUERY_END && self->portal) {
    xdp_portal_session_monitor_query_end_response(self->portal);
  } else if (*state == XDP_LOGIN_SESSION_ENDING) {
    g_application_quit(G_APPLICATION(self));
  }
}

static void on_startup(MendingwallDApplication* self) {
  /* basic initialization */
  self->portal = xdp_portal_initable_new(NULL);
  self->global = g_settings_new("org.indii.mendingwall");

  /* start */
  gboolean themes = g_settings_get_boolean(self->global, "themes");
  gboolean menus = g_settings_get_boolean(self->global, "menus");

  if (themes) {
    watch_themes(self);
    save_themes();
  }
  if (menus) {
    watch_menus(self);
    tidy_menus();
  }
  if (themes || menus) {
    /* keep running as background process */
    g_application_hold(G_APPLICATION(self));

    /* register with portal for session end; this is necessary when systemd
     * (or otherwise) is not configured to kill user processes at end of
     * session; also used for the monitoring of background apps in GNOME */
    if (self->portal) {
      g_signal_connect_swapped(self->portal, "session-state-changed",
          G_CALLBACK(on_session_state_changed), NULL);
    }

    /* watch settings to quit later if disabled */
    g_signal_connect_swapped(self->global, "changed::themes",
        G_CALLBACK(on_changed_themes), self);
    g_signal_connect_swapped(self->global, "changed::menus",
        G_CALLBACK(on_changed_menus), self);
  } else {
    /* quit now */
    g_application_quit(G_APPLICATION(self));
  }
}

static void on_activate(MendingwallDApplication*) {
  //
}

void mendingwall_d_application_dispose(GObject* o) {
  MendingwallDApplication* self = MENDINGWALL_D_APPLICATION(o);

  if (self->portal) {
    g_clear_object(&self->portal);
  }
  if (self->global) {
    g_clear_object(&self->global);
  }
  if (self->theme_settings) {
    g_ptr_array_free(self->theme_settings, TRUE);
    self->theme_settings = NULL;
  }
  if (self->theme_monitors) {
    g_ptr_array_free(self->theme_monitors, TRUE);
    self->theme_monitors = NULL;
  }
  if (self->menu_monitors) {
    g_ptr_array_free(self->menu_monitors, TRUE);
    self->menu_monitors = NULL;
  }

  G_OBJECT_CLASS(mendingwall_d_application_parent_class)->dispose(o);
}

void mendingwall_d_application_finalize(GObject* o) {
  G_OBJECT_CLASS(mendingwall_d_application_parent_class)->finalize(o);
}

void mendingwall_d_application_class_init(MendingwallDApplicationClass* klass) {
  G_OBJECT_CLASS(klass)->dispose = mendingwall_d_application_dispose;
  G_OBJECT_CLASS(klass)->finalize = mendingwall_d_application_finalize;
}

void mendingwall_d_application_init(MendingwallDApplication* self) {
  self->portal = NULL;
  self->global = NULL;
  self->theme_settings = g_ptr_array_new_null_terminated(4, g_object_unref, TRUE);
  self->theme_monitors = g_ptr_array_new_null_terminated(32, g_object_unref, TRUE);
  self->menu_monitors = g_ptr_array_new_null_terminated(8, g_object_unref, TRUE);
}

MendingwallDApplication* mendingwall_d_application_new(void) {
  MendingwallDApplication* self = MENDINGWALL_D_APPLICATION(
      g_object_new(MENDINGWALL_TYPE_D_APPLICATION,
          "application-id", "org.indii.mendingwall.watch",
          "version", PACKAGE_VERSION,
          "flags", G_APPLICATION_DEFAULT_FLAGS,
          NULL));

  /* command-line options */
  GOptionEntry option_entries[] = {
    G_OPTION_ENTRY_NULL
  };
  g_application_set_option_context_summary(G_APPLICATION(self),
      "- mend themes and tidy menus");
  g_application_set_option_context_description(G_APPLICATION(self),
      "For more information see https://mendingwall.indii.org");
  g_application_add_main_option_entries(G_APPLICATION(self), option_entries);

  g_signal_connect(self, "startup", G_CALLBACK(on_startup), NULL);
  g_signal_connect(self, "activate", G_CALLBACK(on_activate), NULL);

  return self;
}

