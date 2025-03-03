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
#include <gio/gsettingsbackend.h>

struct _MendingwallDApplication {
  GApplication parent_instance;

  GDBusProxy* session_manager;
  GDBusProxy* client_private;
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

static void on_query_end_session(gpointer user_data) {
  MendingwallDApplication* self = MENDINGWALL_D_APPLICATION(user_data);
  g_dbus_proxy_call(self->client_private,
      "EndSessionResponse",
      g_variant_new("(bs)", TRUE, ""),
      G_DBUS_CALL_FLAGS_NONE,
      G_MAXINT,
      NULL, NULL, NULL);
}

static void on_end_session(gpointer user_data) {
  MendingwallDApplication* self = MENDINGWALL_D_APPLICATION(user_data);
  g_application_quit(G_APPLICATION(self));
}

static void on_startup(MendingwallDApplication* self) {
  /* setup */
  configure_environment();

  /* basic initialization */
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

    /* watch settings to quit later if disabled */
    g_signal_connect_swapped(self->global, "changed::themes",
        G_CALLBACK(on_changed_themes), self);
    g_signal_connect_swapped(self->global, "changed::menus",
        G_CALLBACK(on_changed_menus), self);

    /* Register with session manager to terminate on logout. This is necessary
     * in case systemd (or otherwise) is not configured to kill all user
     * processes at end of session. An alternative implementation is to have
     * MendingwallDApplication inherit from GtkApplication instead of
     * GApplication and use its session management features, on which this
     * code is based anyway. The downside of doing so is that memory use
     * of the background process increases a lot (it can be ten fold) because
     * of everything else that comes with GtkApplication, such as a GDK
     * surface. */
    const gchar* app_id = "org.indii.mendingwall.watch";
    const gchar* desktop_autostart_id = g_getenv("DESKTOP_AUTOSTART_ID");
    g_autofree const gchar* client_id = g_strdup(desktop_autostart_id ? desktop_autostart_id : "");

    GDBusConnection* dbus = g_application_get_dbus_connection(G_APPLICATION(self));
    if (dbus) {
      /* try GNOME session manager first */
      const char* dbus_name = "org.gnome.SessionManager";
      const char* dbus_path = "/org/gnome/SessionManager";
      const char* dbus_interface = "org.gnome.SessionManager";
      const char* dbus_client_interface = "org.gnome.SessionManager.ClientPrivate";
      self->session_manager = g_dbus_proxy_new_sync(dbus,
          G_DBUS_PROXY_FLAGS_DO_NOT_AUTO_START |
          G_DBUS_PROXY_FLAGS_DO_NOT_LOAD_PROPERTIES |
          G_DBUS_PROXY_FLAGS_DO_NOT_CONNECT_SIGNALS,
          NULL,
          dbus_name,
          dbus_path,
          dbus_interface,
          NULL,
          NULL);
      if (!self->session_manager) {
        /* try Xfce session manager instead */
        dbus_name = "org.xfce.SessionManager";
        dbus_path = "/org/xfce/SessionManager";
        dbus_interface = "org.xfce.SessionManager";
        dbus_client_interface = "org.xfce.Session.Client";
        self->session_manager = g_dbus_proxy_new_sync(dbus,
            G_DBUS_PROXY_FLAGS_DO_NOT_AUTO_START |
            G_DBUS_PROXY_FLAGS_DO_NOT_LOAD_PROPERTIES |
            G_DBUS_PROXY_FLAGS_DO_NOT_CONNECT_SIGNALS,
            NULL,
            dbus_name,
            dbus_path,
            dbus_interface,
            NULL,
            NULL);
      }
      if (self->session_manager) {
        g_autoptr(GVariant) res = g_dbus_proxy_call_sync(
            self->session_manager,
            "RegisterClient",
            g_variant_new("(ss)", app_id, client_id),
            G_DBUS_CALL_FLAGS_NONE,
            G_MAXINT,
            NULL,
            NULL);
        if (res) {
          g_autofree const gchar* dbus_client_path = NULL;
          g_variant_get(res, "(o)", &dbus_client_path);
          if (dbus_path) {
            /* quit on session end */
            self->client_private = g_dbus_proxy_new_sync(dbus,
                G_DBUS_PROXY_FLAGS_NONE,
                NULL,
                dbus_name,
                dbus_client_path,
                dbus_client_interface,
                NULL,
                NULL);
            if (self->client_private) {
              g_signal_connect_swapped(
                  self->client_private,
                  "g-signal::QueryEndSession",
                  G_CALLBACK(on_query_end_session),
                  self);
              g_signal_connect_swapped(
                  self->client_private,
                  "g-signal::EndSession",
                  G_CALLBACK(on_end_session),
                  self);
            }
          }
        }
      }
    }
  } else {
    /* quit now */
    g_application_quit(G_APPLICATION(self));
  }
}

static void on_activate(MendingwallDApplication* self) {
  //
}

void mendingwall_d_application_dispose(GObject* o) {
  MendingwallDApplication* self = MENDINGWALL_D_APPLICATION(o);

  if (self->session_manager) {
    g_clear_object(&self->session_manager);
  }
  if (self->client_private) {
    g_clear_object(&self->client_private);
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
  self->session_manager = NULL;
  self->client_private = NULL;
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

