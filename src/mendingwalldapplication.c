#include <config.h>
#include <mendingwalldapplication.h>

#define G_SETTINGS_ENABLE_BACKEND 1
#include <gio/gio.h>
#include <gio/gsettingsbackend.h>

/**
 * A tidier way to loop. Use as foreach(value, values) { ... }.
 */
#define foreach(value, values) \
    typeof(values) values##_iter_ = values; \
    for (typeof(*values) value; (value = *values##_iter_); ++values##_iter_)

struct _MendingwallDApplication {
  MendingwallDaemon parent_instance;
  GSettings* global;
  const gchar* desktop;
  GKeyFile* theme_config;
  GPtrArray* theme_settings;
  GPtrArray* theme_files;
  GPtrArray* theme_monitors;
  GKeyFile* menu_config;
  GPtrArray* menu_dirs;
  GPtrArray* menu_monitors;
};

typedef struct {
  gboolean restore, watch;
} MendingwallDOptions;

G_DEFINE_TYPE(MendingwallDApplication, mendingwalld_application, MENDINGWALL_TYPE_DAEMON)

static void save_settings(const gchar* desktop, GSettings* settings) {
  g_autoptr(GSettingsSchema) schema = NULL;
  g_object_get(settings, "settings-schema", &schema, NULL);

  const gchar* id = g_settings_schema_get_id(schema);
  g_autofree gchar* filename = g_strconcat(g_get_user_data_dir(), "/mendingwall/save/", desktop, ".gsettings", NULL);

  g_autoptr(GSettingsBackend) backend = g_keyfile_settings_backend_new(filename, "/", NULL);
  g_autoptr(GSettings) save = g_settings_new_with_backend(id, backend);
  g_auto(GStrv) keys = g_settings_schema_list_keys(schema);
  foreach(key, keys) {
    g_autoptr(GVariant) value = g_settings_get_value(settings, key);
    g_settings_set_value(save, key, value);
  }
}

static void restore_settings(const gchar* desktop, GSettings* settings) {
  /* schema */
  g_autoptr(GSettingsSchema) schema = NULL;
  g_object_get(settings, "settings-schema", &schema, NULL);

  /* saved file */
  const gchar* id = g_settings_schema_get_id(schema);
  g_autofree gchar* filename = g_strconcat(g_get_user_data_dir(), "/mendingwall/save/", desktop, ".gsettings", NULL);
  g_autoptr(GFile) file = g_file_new_for_path(filename);
  g_auto(GStrv) keys = g_settings_schema_list_keys(schema);
  if (g_file_query_exists(file, NULL)) {
    /* saved file exists, restore settings */
    g_autoptr(GSettingsBackend) backend = g_keyfile_settings_backend_new(filename, "/", NULL);
    g_autoptr(GSettings) saved = g_settings_new_with_backend(id, backend);
    foreach(key, keys) {
      g_autoptr(GVariant) value = g_settings_get_value(saved, key);
      g_settings_set_value(settings, key, value);
    }
  } else {
    /* if the saved file does not exist, restore settings to their defaults;
     * this is deliberate, it means that when starting a new desktop
     * environment for the first time after running some other desktop
     * environment, existing settings are reset, the user's settings look
     * pristine, and the new desktop environment performs its default initial
     * setup */
    foreach(key, keys) {
      g_settings_reset(settings, key);
    }
  }
}

static void save_file(const gchar* desktop, GFile* file) {
  g_autoptr(GFile) rel_to = g_file_new_for_path(g_get_user_config_dir());
  g_autofree char* rel = g_file_get_relative_path(rel_to, file);
  g_autoptr(GFile) saved = g_file_new_build_filename(g_get_user_data_dir(), "mendingwall", "save", desktop, rel, NULL);
  if (g_file_query_exists(file, NULL)) {
    /* save file */
    g_autoptr(GFile) parent = g_file_get_parent(saved);
    g_file_make_directory_with_parents(parent, NULL, NULL);
    g_file_copy(file, saved, G_FILE_COPY_OVERWRITE|G_FILE_COPY_ALL_METADATA, NULL, NULL, NULL, NULL);
  } else {
    /* delete any existing saved file; it does not exist in the current
     * configuration */
    g_file_delete(saved, NULL, NULL);
  }
}

static void restore_file(const gchar* desktop, GFile* to) {
  g_autoptr(GFile) rel_to = g_file_new_for_path(g_get_user_config_dir());
  g_autofree char* rel = g_file_get_relative_path(rel_to, to);
  g_autoptr(GFile) from = g_file_new_build_filename(g_get_user_data_dir(), "mendingwall", "save", desktop, rel, NULL);
  if (g_file_query_exists(from, NULL)) {
    /* restore file */
    g_file_copy(from, to, G_FILE_COPY_OVERWRITE|G_FILE_COPY_ALL_METADATA, NULL, NULL, NULL, NULL);
  } else {
    /* delete file; it does not exist in the desired configuraiton; this
     * happens either because the file is not part of the save, or there is no
     * save; the latter occurs when starting a new desktop environment for the
     * first time after running some other desktop environment, and by
     * deleting all config files, the user's home directory looks pristine,
     * and the new desktop environment performs its default initial setup */
    g_file_delete(to, NULL, NULL);
  }
}

static void update_app(GKeyFile* config, const char* basename, GKeyFile* app_file) {
  g_auto(GStrv) only_show_in = g_key_file_get_string_list(config, basename, "OnlyShowIn", NULL, NULL);
  g_auto(GStrv) not_show_in = g_key_file_get_string_list(config, basename, "NotShowIn", NULL, NULL);
  if (only_show_in) {
    g_key_file_set_string_list(app_file, "Desktop Entry", "OnlyShowIn", (const gchar* const*)only_show_in, g_strv_length(only_show_in));
  }
  if (not_show_in) {
    g_key_file_set_string_list(app_file, "Desktop Entry", "NotShowIn", (const gchar* const*)not_show_in, g_strv_length(not_show_in));
  }
}

static void tidy_app(GKeyFile* config, const char* basename) {
  /* load desktop entry file, if it exists */
  g_autofree char* app_path = g_build_filename("applications", basename, NULL);
  g_autoptr(GKeyFile) app_file = g_key_file_new();
  if (g_key_file_load_from_data_dirs(app_file, app_path, NULL, G_KEY_FILE_KEEP_COMMENTS|G_KEY_FILE_KEEP_TRANSLATIONS, NULL)) {
    /* update app file */
    update_app(config, basename, app_file);

    /* save to user's data directory, if it does not already exist there */
    g_autofree gchar* to_path = g_build_filename(g_get_user_data_dir(), "applications", basename, NULL);
    g_autoptr(GFile) to_file = g_file_new_for_path(to_path);
    if (!g_file_query_exists(to_file, NULL)) {
      g_key_file_save_to_file(app_file, to_path, NULL);
    }
  }
}

static void untidy_app(GKeyFile* config, const char* basename) {
  /* load desktop entry file, if it exists */
  g_autofree char* app_path = g_build_filename("applications", basename, NULL);
  g_autoptr(GKeyFile) app_file = g_key_file_new();
  if (g_key_file_load_from_data_dirs(app_file, app_path, NULL, G_KEY_FILE_KEEP_COMMENTS|G_KEY_FILE_KEEP_TRANSLATIONS, NULL)) {
    /* update app file */
    update_app(config, basename, app_file);

    /* check if a matching desktop entry file exists in the user's data
     * directory; if so and its contents match what would be written, delete
     * it, otherwise custom changes have been made so leave it */
    g_autofree gchar* to_path = g_build_filename(g_get_user_data_dir(), app_path, NULL);
    g_autoptr(GFile) to_file = g_file_new_for_path(to_path);
    if (g_file_query_exists(to_file, NULL)) {
      g_autofree gchar* app_data = g_key_file_to_data(app_file, NULL, NULL);
      g_autofree gchar* to_data;
      g_file_load_contents(to_file, NULL, &to_data, NULL, NULL, NULL);
      if (g_str_equal(app_data, to_data)) {
        g_file_delete(to_file, NULL, NULL);
      }
    }
  }
}

static void on_changed_settings(MendingwallDApplication* self, GSettings* settings) {
  save_settings(self->desktop, settings);
}

static void on_changed_file(MendingwallDApplication* self, GFile* file) {
  save_file(self->desktop, file);
}

static void on_changed_app(MendingwallDApplication* self, GFile* app) {
  g_autofree char* basename = g_file_get_basename(app);
  tidy_app(self->menu_config, basename);
}

static void save_themes(MendingwallDApplication* self) {
  /* save settings */
  g_auto(GStrv) schemas = g_key_file_get_string_list(self->theme_config, self->desktop, "GSettings", NULL, NULL);
  foreach(schema, schemas) {
    g_autoptr(GSettings) settings = g_settings_new(schema);
    save_settings(self->desktop, settings);
  }

  /* save config files */
  g_auto(GStrv) paths = g_key_file_get_string_list(self->theme_config, self->desktop, "ConfigFiles", NULL, NULL);
  foreach(path, paths) {
    g_autoptr(GFile) file = g_file_new_build_filename(g_get_user_config_dir(), path, NULL);
    save_file(self->desktop, file);
  }
}

static void restore_themes(MendingwallDApplication* self) {
  /* restore settings */
  g_auto(GStrv) schemas = g_key_file_get_string_list(self->theme_config, self->desktop, "GSettings", NULL, NULL);
  foreach(schema, schemas) {
    g_autoptr(GSettings) settings = g_settings_new(schema);
    restore_settings(self->desktop, settings);
  }

  /* restore config files */
  g_auto(GStrv) paths = g_key_file_get_string_list(self->theme_config, self->desktop, "ConfigFiles", NULL, NULL);
  foreach(path, paths) {
    g_autoptr(GFile) file = g_file_new_build_filename(g_get_user_config_dir(), path, NULL);
    restore_file(self->desktop, file);
  }
}

static void tidy_menus(MendingwallDApplication* self) {
  g_auto(GStrv) basenames = g_key_file_get_groups(self->menu_config, NULL);
  foreach(basename, basenames) {
    tidy_app(self->menu_config, basename);
  }
}

static void untidy_menus(MendingwallDApplication* self) {
  g_auto(GStrv) basenames = g_key_file_get_groups(self->menu_config, NULL);
  foreach(basename, basenames) {
    untidy_app(self->menu_config, basename);
  }
}

static void watch_themes(MendingwallDApplication* self) {
  /* save settings and watch */
  g_auto(GStrv) schemas = g_key_file_get_string_list(self->theme_config, self->desktop, "GSettings", NULL, NULL);
  foreach(schema, schemas) {
    GSettings* settings = g_settings_new(schema);
    g_signal_connect_swapped(settings, "change-event", G_CALLBACK(on_changed_settings), self);

    g_ptr_array_add(self->theme_settings, settings);
  }

  /* save config files and watch */
  g_auto(GStrv) paths = g_key_file_get_string_list(self->theme_config, self->desktop, "ConfigFiles", NULL, NULL);
  foreach(path, paths) {
    GFile* file = g_file_new_build_filename(g_get_user_config_dir(), path, NULL);
    GFileMonitor* monitor = g_file_monitor_file(file, G_FILE_MONITOR_NONE, NULL, NULL);
    g_signal_connect_swapped(monitor, "changed", G_CALLBACK(on_changed_file), self);

    g_ptr_array_add(self->theme_files, file);
    g_ptr_array_add(self->theme_monitors, monitor);
  }
}

static void unwatch_themes(MendingwallDApplication* self) {
  g_ptr_array_set_size(self->theme_monitors, 0);
  g_ptr_array_set_size(self->theme_files, 0);
  g_ptr_array_set_size(self->theme_settings, 0);
}

static void watch_menus(MendingwallDApplication* self) {
  /* watch application directories */
  const gchar** paths = (const gchar**)g_get_system_data_dirs();
  foreach(path, paths) {
    GFile* dir = g_file_new_build_filename(path, "applications", NULL);
    GFileMonitor* monitor = g_file_monitor_directory(dir, G_FILE_MONITOR_NONE, NULL, NULL);
    g_signal_connect_swapped(monitor, "changed", G_CALLBACK(on_changed_app), self);

    g_ptr_array_add(self->menu_dirs, dir);
    g_ptr_array_add(self->menu_monitors, monitor);
  }
}

static void unwatch_menus(MendingwallDApplication* self) {
  g_ptr_array_set_size(self->menu_monitors, 0);
  g_ptr_array_set_size(self->menu_dirs, 0);
}

static void on_changed(MendingwallDApplication* self) {
  gboolean themes_enabled = g_settings_get_boolean(self->global, "themes");
  gboolean menus_enabled = g_settings_get_boolean(self->global, "menus");

  if (themes_enabled) {
    restore_themes(self);
    watch_themes(self);
  } else {
    unwatch_themes(self);
    save_themes(self);
  }
  if (menus_enabled) {
    tidy_menus(self);
    watch_menus(self);
  } else {
    unwatch_menus(self);
    untidy_menus(self);
  }

  if (!themes_enabled && !menus_enabled) {
    g_application_quit(G_APPLICATION(self));
  }
}

static void on_activate(MendingwallDApplication* self, MendingwallDOptions* options) {
  mendingwall_daemon_activate(MENDINGWALL_DAEMON(self));

  if (options->restore) {
    restore_themes(self);
  }

  gboolean themes_enabled = g_settings_get_boolean(self->global, "themes");
  gboolean menus_enabled = g_settings_get_boolean(self->global, "menus");
  if (options->watch && (themes_enabled || menus_enabled)) {
    if (themes_enabled) {
      watch_themes(self);
    }
    if (menus_enabled) {
      watch_menus(self);
    }

    /* quit later */
    g_signal_connect_swapped(self->global, "changed", G_CALLBACK(on_changed), self);
  } else {
    /* quit now */
    g_application_quit(G_APPLICATION(self));
  }
}

void mendingwalld_application_dispose(GObject* o) {
  MendingwallDApplication* self = MENDINGWALL_D_APPLICATION(o);

  g_object_unref(self->global);
  g_key_file_free(self->theme_config);
  g_ptr_array_free(self->theme_settings, TRUE);
  g_ptr_array_free(self->theme_files, TRUE);
  g_ptr_array_free(self->theme_monitors, TRUE);
  g_key_file_free(self->menu_config);
  g_ptr_array_free(self->menu_dirs, TRUE);
  g_ptr_array_free(self->menu_monitors, TRUE);

  G_OBJECT_CLASS(mendingwalld_application_parent_class)->dispose(o);
}

void mendingwalld_application_finalize(GObject* o) {
  G_OBJECT_CLASS(mendingwalld_application_parent_class)->finalize(o);
}

void mendingwalld_application_class_init(MendingwallDApplicationClass* klass) {
  G_OBJECT_CLASS(klass)->dispose = mendingwalld_application_dispose;
  G_OBJECT_CLASS(klass)->finalize = mendingwalld_application_finalize;
}

void mendingwalld_application_init(MendingwallDApplication* self) {
  self->global = g_settings_new("org.indii.mendingwall");
  self->desktop = g_getenv("XDG_CURRENT_DESKTOP");
  self->theme_config = g_key_file_new();
  self->theme_settings = g_ptr_array_new_with_free_func(g_object_unref);
  self->theme_files = g_ptr_array_new_with_free_func(g_object_unref);
  self->theme_monitors = g_ptr_array_new_with_free_func(g_object_unref);
  self->menu_config = g_key_file_new();
  self->menu_dirs = g_ptr_array_new_with_free_func(g_object_unref);
  self->menu_monitors = g_ptr_array_new_with_free_func(g_object_unref);

  /* check desktop */
  if (!self->desktop) {
    g_printerr("Environment variable XDG_CURRENT_DESKTOP is not set\n");
    exit(1);
  }

  /* load themes config file */
  if (!g_key_file_load_from_data_dirs(self->theme_config, "mendingwall/themes.conf", NULL, G_KEY_FILE_NONE, NULL)) {
    g_printerr("Cannot find config file mendingwall/themes.conf\n");
    exit(1);
  }
  if (!g_key_file_has_group(self->theme_config, self->desktop)) {
    g_printerr("Desktop environment %s is not supported\n", self->desktop);
    exit(1);
  }

  /* load menues config file */
  if (!g_key_file_load_from_data_dirs(self->menu_config, "mendingwall/menus.conf", NULL, G_KEY_FILE_NONE, NULL)) {
    g_printerr("Cannot find config file mendingwall/menus.conf\n");
    exit(1);
  }
}

MendingwallDApplication* mendingwalld_application_new(void) {
  MendingwallDApplication* self = MENDINGWALL_D_APPLICATION(
      g_object_new(MENDINGWALL_TYPE_D_APPLICATION,
          "application-id", "org.indii.mendingwalld",
          "version", PACKAGE_VERSION,
          "flags", G_APPLICATION_DEFAULT_FLAGS,
          NULL));

  /* command-line options */
  MendingwallDOptions options;
  options.restore = FALSE;
  options.watch = FALSE;
  GOptionEntry option_entries[] = {
    { "restore", 0, 0, G_OPTION_ARG_NONE, &options.restore, "Restore theme configuration at initialization", NULL },
    { "watch", 0, 0, G_OPTION_ARG_NONE, &options.watch, "Continue to watch for changes and save", NULL },
    G_OPTION_ENTRY_NULL
  };
  g_application_set_option_context_summary(G_APPLICATION(self), "- untangle themes");
  g_application_set_option_context_description(G_APPLICATION(self), "For more information see https://mendingwall.org");
  g_application_add_main_option_entries(G_APPLICATION(self), option_entries);

  g_signal_connect(self, "activate", G_CALLBACK(on_activate), &options);

  return self;
}
