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
  GFile* config_dir;
  GSettingsBackend* settings_backend;
  char* save_path;

  GPtrArray* theme_settings;
  GPtrArray* theme_files;
  GPtrArray* theme_monitors;

  GKeyFile* menus_config;
  GPtrArray* menu_dirs;
  GPtrArray* menu_monitors;
};

G_DEFINE_TYPE(MendingwallDApplication, mendingwall_d_application, G_TYPE_APPLICATION)

static void save_setting(MendingwallDApplication* self, GSettings* settings,
    gchar* key) {
  /* get settings schema */
  g_autoptr(GSettingsSchema) schema = NULL;
  g_object_get(settings, "settings-schema", &schema, NULL);
  const gchar* schema_id = g_settings_schema_get_id(schema);
  g_autoptr(GSettings) saved = g_settings_new_with_backend(schema_id,
      self->settings_backend);
  g_autoptr(GVariant) value = g_settings_get_value(settings, key);
  g_settings_set_value(saved, key, value);
}

static void save_settings(MendingwallDApplication* self, GSettings* settings) {
  /* get settings schema */
  g_autoptr(GSettingsSchema) schema = NULL;
  g_object_get(settings, "settings-schema", &schema, NULL);
  const gchar* schema_id = g_settings_schema_get_id(schema);
  g_auto(GStrv) keys = g_settings_schema_list_keys(schema);

  /* save settings to file backend */
  g_autoptr(GSettings) saved = g_settings_new_with_backend(schema_id,
      self->settings_backend);
  g_settings_delay(saved);
  foreach(key, keys) {
    g_autoptr(GVariant) value = g_settings_get_value(settings, key);
    g_settings_set_value(saved, key, value);
  }
  g_settings_apply(saved);
}

static void save_file(MendingwallDApplication* self, GFile* file) {
  g_autofree char* rel = g_file_get_relative_path(self->config_dir, file);
  g_autoptr(GFile) saved = g_file_new_build_filename(self->save_path, rel, NULL);
  if (g_file_query_exists(file, NULL)) {
    /* save file */
    g_autoptr(GFile) parent = g_file_get_parent(saved);
    g_file_make_directory_with_parents(parent, NULL, NULL);
    g_file_copy_async(file, saved, G_FILE_COPY_OVERWRITE|G_FILE_COPY_ALL_METADATA,
        G_PRIORITY_DEFAULT, NULL, NULL, NULL, NULL, NULL);
  } else {
    /* delete any existing saved file; it does not exist in the current
     * configuration */
    g_file_delete_async(saved, G_PRIORITY_DEFAULT, NULL, NULL, NULL);
  }
}

static gboolean update_app(MendingwallDApplication* self,
    const char* basename, GKeyFile* app_file) {
  /* update OnlyShowIn */
  g_auto(GStrv) only_show_in = g_key_file_get_string_list(
      self->menus_config, basename, "OnlyShowIn", NULL, NULL);
  if (only_show_in) {
    g_key_file_set_string_list(app_file, "Desktop Entry", "OnlyShowIn",
        (const gchar* const*)only_show_in, g_strv_length(only_show_in));
  }

  /* update NotShowIn */
  g_auto(GStrv) not_show_in = g_key_file_get_string_list(
      self->menus_config, basename, "NotShowIn", NULL, NULL);
  if (not_show_in) {
    g_key_file_set_string_list(app_file, "Desktop Entry", "NotShowIn",
        (const gchar* const*)not_show_in, g_strv_length(not_show_in));
  }

  /* add custom marker */
  g_key_file_set_boolean(app_file, "Desktop Entry", "X-MendingWall-Tidy",
      TRUE);

  return only_show_in || not_show_in;
}

static void tidy_app(MendingwallDApplication* self, const char* basename) {
  /* load desktop entry file, if it exists */
  g_autofree char* app_path = g_build_filename("applications", basename, NULL);
  g_autoptr(GKeyFile) app_file = g_key_file_new();
  if (g_key_file_load_from_data_dirs(app_file, app_path, NULL,
      G_KEY_FILE_KEEP_COMMENTS|G_KEY_FILE_KEEP_TRANSLATIONS, NULL)) {
    /* update app file */
    if (update_app(self, basename, app_file)) {
      /* save to user's data directory, if it does not already exist there */
      g_autofree gchar* to_path = g_build_filename(g_get_user_data_dir(),
          "applications", basename, NULL);
      g_autoptr(GFile) to_file = g_file_new_for_path(to_path);
      if (!g_file_query_exists(to_file, NULL)) {
        g_autoptr(GFile) to_dir = g_file_new_build_filename(
            g_get_user_data_dir(), "applications", NULL);
        g_file_make_directory_with_parents(to_dir, NULL, NULL);
        g_key_file_save_to_file(app_file, to_path, NULL);
      }
    }
  }
}

static void untidy_app(MendingwallDApplication* self, const char* basename) {
  /* load desktop entry file, if it exists */
  g_autofree char* app_path = g_build_filename("applications", basename, NULL);
  g_autoptr(GKeyFile) app_file = g_key_file_new();
  if (g_key_file_load_from_data_dirs(app_file, app_path, NULL,
      G_KEY_FILE_KEEP_COMMENTS|G_KEY_FILE_KEEP_TRANSLATIONS, NULL)) {
    /* update app file */
    if (update_app(self, basename, app_file)) {
      /* check if a matching desktop entry file exists in the user's data
      * directory; if so and its contents match what would be written, delete
      * it, otherwise custom changes have been made so leave it */
      g_autofree gchar* to_path = g_build_filename(g_get_user_data_dir(),
          app_path, NULL);
      g_autoptr(GFile) to_file = g_file_new_for_path(to_path);
      if (g_file_query_exists(to_file, NULL)) {
        g_autofree gchar* app_data = g_key_file_to_data(app_file, NULL, NULL);
        g_autofree gchar* to_data = NULL;
        g_file_load_contents(to_file, NULL, &to_data, NULL, NULL, NULL);
        if (g_str_equal(app_data, to_data)) {
          g_file_delete_async(to_file, G_PRIORITY_DEFAULT, NULL, NULL, NULL);
        }
      }
    }
  }
}

static void on_changed_setting(gpointer user_data, gchar* key,
    GSettings* settings) {
  MendingwallDApplication* self = MENDINGWALL_D_APPLICATION(user_data);
  save_setting(self, settings, key);
}

static void on_changed_file(gpointer user_data, GFile* file) {
  MendingwallDApplication* self = MENDINGWALL_D_APPLICATION(user_data);
  save_file(self, file);
}

static void on_changed_app(gpointer user_data, GFile* app_file) {
  MendingwallDApplication* self = MENDINGWALL_D_APPLICATION(user_data);
  g_autofree char* basename = g_file_get_basename(app_file);
  tidy_app(self, basename);
}

static void save_themes(MendingwallDApplication* self) {
  /* save settings */
  foreach (settings, (GSettings**)self->theme_settings->pdata) {
    save_settings(self, settings);
  }

  /* save config files */
  foreach(file, (GFile**)self->theme_files->pdata) {
    save_file(self, file);
  }
}

static void tidy_menus(MendingwallDApplication* self) {
  g_auto(GStrv) basenames = g_key_file_get_groups(self->menus_config, NULL);
  foreach(basename, basenames) {
    tidy_app(self, basename);
  }
}

static void untidy_menus(MendingwallDApplication* self) {
  g_auto(GStrv) basenames = g_key_file_get_groups(self->menus_config, NULL);
  foreach(basename, basenames) {
    untidy_app(self, basename);
  }
}

static void watch_themes(MendingwallDApplication* self) {
  /* watch settings */
  foreach (settings, (GSettings**)self->theme_settings->pdata) {
    g_signal_connect_swapped(settings, "changed",
        G_CALLBACK(on_changed_setting), self);
  }

  /* watch config files */
  foreach (file, (GFile**)self->theme_files->pdata) {
    GFileMonitor* monitor = g_file_monitor_file(file, G_FILE_MONITOR_NONE,
        NULL, NULL);
    g_signal_connect_swapped(monitor, "changed", G_CALLBACK(on_changed_file),
        self);
    g_ptr_array_add(self->theme_monitors, monitor);
  }
}

static void unwatch_themes(MendingwallDApplication* self) {
  /* unwatch settings */
  foreach (settings, (GSettings**)self->theme_settings->pdata) {
    g_signal_handlers_disconnect_by_data(settings, self);
  }

  /* unwatch config files */
  g_ptr_array_set_size(self->theme_monitors, 0);
}

static void watch_menus(MendingwallDApplication* self) {
  /* watch application directories */
  foreach (dir, (GFile**)self->menu_dirs->pdata) {
    GFileMonitor* monitor = g_file_monitor_directory(dir, G_FILE_MONITOR_NONE,
        NULL, NULL);
    g_signal_connect_swapped(monitor, "changed", G_CALLBACK(on_changed_app),
        self);
    g_ptr_array_add(self->menu_monitors, monitor);
  }
}

static void unwatch_menus(MendingwallDApplication* self) {
  /* unwatch application directories */
  g_ptr_array_set_size(self->menu_monitors, 0);
}

static void on_changed_themes(gpointer user_data) {
  MendingwallDApplication* self = MENDINGWALL_D_APPLICATION(user_data);
  gboolean themes = g_settings_get_boolean(self->global, "themes");
  gboolean menus = g_settings_get_boolean(self->global, "menus");
  if (themes) {
    watch_themes(self);
    save_themes(self);
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
    tidy_menus(self);
  } else {
    unwatch_menus(self);
    untidy_menus(self);
  }
  if (!themes && !menus) {
    g_application_quit(G_APPLICATION(self));
  }
}

static void on_session_state_changed(MendingwallDApplication* self, gboolean,
  XdpLoginSessionState* state) {
  if (*state == XDP_LOGIN_SESSION_QUERY_END) {
    xdp_portal_session_monitor_query_end_response(self->portal);
  } else if (*state == XDP_LOGIN_SESSION_ENDING) {
    g_application_quit(G_APPLICATION(self));
  }
}

static void on_startup(MendingwallDApplication* self) {
  /* current desktop */
  const char* desktop = g_getenv("XDG_CURRENT_DESKTOP");
  if (!desktop) {
    g_printerr("Environment variable XDG_CURRENT_DESKTOP is not set\n");
    exit(1);
  }

  /* path to save settings */
  g_autofree char* settings_save_path = g_strconcat(g_get_user_data_dir(),
      "/", "mendingwall", "/", "save", "/", desktop, ".gsettings", NULL);

  /* basic initialization */
  self->portal = xdp_portal_new();
  self->global = g_settings_new("org.indii.mendingwall");
  self->config_dir = g_file_new_for_path(g_get_user_config_dir());
  self->settings_backend = g_keyfile_settings_backend_new(settings_save_path,
      "/", NULL);
  self->save_path = g_strconcat(g_get_user_data_dir(), "/", "mendingwall",
      "/", "save", "/", desktop, NULL);

  const guint reserved = 8;
  self->theme_settings = g_ptr_array_new_null_terminated(reserved, g_object_unref, TRUE);
  self->theme_files = g_ptr_array_new_null_terminated(reserved, g_object_unref, TRUE);
  self->theme_monitors = g_ptr_array_new_null_terminated(reserved, g_object_unref, TRUE);
  self->menus_config = g_key_file_new();
  self->menu_dirs = g_ptr_array_new_null_terminated(reserved, g_object_unref, TRUE);
  self->menu_monitors = g_ptr_array_new_null_terminated(reserved, g_object_unref, TRUE);

  /* load themes config file */
  g_autoptr(GKeyFile) themes_config = g_key_file_new();
  if (!g_key_file_load_from_data_dirs(themes_config,
      "mendingwall/themes.conf", NULL, G_KEY_FILE_NONE, NULL)) {
    g_printerr("Cannot find config file mendingwall/themes.conf\n");
    exit(1);
  }
  if (!g_key_file_has_group(themes_config, desktop)) {
    g_printerr("Desktop environment %s is not supported\n", desktop);
    exit(1);
  }

  /* populate settings to save */
  g_auto(GStrv) schema_ids = g_key_file_get_string_list(themes_config,
      desktop, "GSettings", NULL, NULL);
  foreach(schema_id, schema_ids) {
    GSettings* settings = g_settings_new(schema_id);
    g_ptr_array_add(self->theme_settings, settings);
  }

  /* populate config files to save */
  g_auto(GStrv) paths = g_key_file_get_string_list(themes_config, desktop,
      "ConfigFiles", NULL, NULL);
  foreach(path, paths) {
    GFile* file = g_file_new_build_filename(g_get_user_config_dir(), path,
        NULL);
    g_ptr_array_add(self->theme_files, file);
  }

  /* load menus config file */
  if (!g_key_file_load_from_data_dirs(self->menus_config,
      "mendingwall/menus.conf", NULL, G_KEY_FILE_NONE, NULL)) {
    g_printerr("Cannot find config file mendingwall/menus.conf\n");
    exit(1);
  }

  /* populate directories with application desktop entries */
  foreach(path, (const gchar**)g_get_system_data_dirs()) {
    GFile* dir = g_file_new_build_filename(path, "applications",
        NULL);
    g_ptr_array_add(self->menu_dirs, dir);
  }

  /* start */
  gboolean themes = g_settings_get_boolean(self->global, "themes");
  gboolean menus = g_settings_get_boolean(self->global, "menus");

  if (themes) {
    watch_themes(self);
    save_themes(self);
  }
  if (menus) {
    watch_menus(self);
    tidy_menus(self);
  }
  if (themes || menus) {
    /* keep running as background process */
    g_application_hold(G_APPLICATION(self));

    /* register with portal for session end; this is necessary when systemd
     * (or otherwise) is not configured to kill user processes at end of
     * session; also used for the monitoring of background apps in GNOME */
    g_signal_connect_swapped(self->portal, "session-state-changed",
        G_CALLBACK(on_session_state_changed), NULL);

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

static void on_activate(MendingwallDApplication* self) {
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
  if (self->config_dir) {
    g_clear_object(&self->config_dir);
  }
  if (self->settings_backend) {
    g_clear_object(&self->settings_backend);
  }
  if (self->save_path) {
    g_free(self->save_path);
    self->save_path = NULL;
  }
  if (self->theme_settings) {
    g_ptr_array_free(self->theme_settings, TRUE);
    self->theme_settings = NULL;
  }
  if (self->theme_files) {
    g_ptr_array_free(self->theme_files, TRUE);
    self->theme_files = NULL;
  }
  if (self->theme_monitors) {
    g_ptr_array_free(self->theme_monitors, TRUE);
    self->theme_monitors = NULL;
  }
  if (self->menus_config) {
    g_key_file_free(self->menus_config);
    self->menus_config = NULL;
  }
  if (self->menu_dirs) {
    g_ptr_array_free(self->menu_dirs, TRUE);
    self->menu_dirs = NULL;
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
  self->config_dir = NULL;
  self->settings_backend = NULL;
  self->save_path = NULL;
  self->theme_settings = NULL;
  self->theme_files = NULL;
  self->theme_monitors = NULL;
  self->menus_config = NULL;
  self->menu_dirs = NULL;
  self->menu_monitors = NULL;
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
