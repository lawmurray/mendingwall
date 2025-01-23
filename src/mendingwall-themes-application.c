#include <config.h>
#include <mendingwall-themes-application.h>

#define G_SETTINGS_ENABLE_BACKEND 1
#include <gio/gio.h>
#include <gio/gsettingsbackend.h>

struct _MendingwallThemesApplication {
  MendingwallBackgroundApplication parent_instance;
  GSettings* global;
  GKeyFile* config;
  GPtrArray* settings;
  GPtrArray* files;
  GPtrArray* monitors;
  const gchar* desktop;
  gboolean save, restore, watch;
};

G_DEFINE_TYPE(MendingwallThemesApplication, mendingwall_themes_application, MENDINGWALL_TYPE_BACKGROUND_APPLICATION)

static void save_settings(GSettings* settings) {
  GSettingsSchema* schema = NULL;
  g_object_get(settings, "settings-schema", &schema, NULL);

  const gchar* id = g_settings_schema_get_id(schema);
  const gchar* desktop = g_getenv("XDG_CURRENT_DESKTOP");
  g_autofree gchar* filename = g_strconcat(g_get_user_data_dir(), "/mendingwall/save/", desktop, ".gsettings", NULL);

  g_autoptr(GSettingsBackend) backend = g_keyfile_settings_backend_new(filename, "/", NULL);
  g_autoptr(GSettings) save = g_settings_new_with_backend(id, backend);

  gchar** keys = g_settings_schema_list_keys(schema);
  for (gchar** key = keys; key && *key; ++key) {
    g_autoptr(GVariant) value = g_settings_get_value(settings, *key);
    g_settings_set_value(save, *key, value);
  }
  g_strfreev(keys);
}

static void restore_settings(GSettings* settings) {
  /* schema */
  GSettingsSchema* schema = NULL;
  g_object_get(settings, "settings-schema", &schema, NULL);

  /* saved file */
  const gchar* id = g_settings_schema_get_id(schema);
  const gchar* desktop = g_getenv("XDG_CURRENT_DESKTOP");
  g_autofree gchar* filename = g_strconcat(g_get_user_data_dir(), "/mendingwall/save/", desktop, ".gsettings", NULL);
  g_autoptr(GFile) file = g_file_new_for_path(filename);

  gchar** keys = g_settings_schema_list_keys(schema);
  if (g_file_query_exists(file, NULL)) {
    /* saved file exists, restore settings */
    g_autoptr(GSettingsBackend) backend = g_keyfile_settings_backend_new(filename, "/", NULL);
    g_autoptr(GSettings) saved = g_settings_new_with_backend(id, backend);
    for (gchar** key = keys; key && *key; ++key) {
      g_autoptr(GVariant) value = g_settings_get_value(saved, *key);
      g_settings_set_value(settings, *key, value);
    }
  } else {
    /* if the saved file does not exist, restore settings to their defaults;
     * this is deliberate, it means that when starting a new desktop
     * environment for the first time after running some other desktop
     * environment, existing settings are reset, the user's settings look
     * pristine, and the new desktop environment performs its default initial
     * setup */
    for (gchar** key = keys; key && *key; ++key) {
      g_settings_reset(settings, *key);
    }
  }
  g_strfreev(keys);
}

static void save_file(GFile* file) {
  const gchar* desktop = g_getenv("XDG_CURRENT_DESKTOP");
  g_autoptr(GFile) rel_to = g_file_new_for_path(g_get_user_config_dir());
  g_autofree char* rel = g_file_get_relative_path(rel_to, file);
  g_autoptr(GFile) saved = g_file_new_build_filename(g_get_user_data_dir(), "mendingwall", "save", desktop, rel, NULL);
  g_autoptr(GFile) parent = g_file_get_parent(saved);
  if (g_file_query_exists(file, NULL)) {
    /* save file */
    g_file_make_directory_with_parents(parent, NULL, NULL);
    g_file_copy(file, saved, G_FILE_COPY_OVERWRITE|G_FILE_COPY_ALL_METADATA, NULL, NULL, NULL, NULL);
  } else {
    /* delete any existing saved file; it does not exist in the current
     * configuration */
    g_file_delete(saved, NULL, NULL);
  }
}

static void restore_file(GFile* to) {
  const gchar* desktop = g_getenv("XDG_CURRENT_DESKTOP");
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

static void changed_settings(GSettings* settings) {
  save_settings(settings);
}

static void changed_file(GFile* file) {
  save_file(file);
}

static gint handle_local_options(MendingwallThemesApplication* self) {
  if ((!self->save && !self->restore) || (self->save && self->restore)) {
    g_print("Error: one of --save or --restore (but not both) required\n");
    return 1;
  } else {
    return -1;
  }
}

static void deactivate(MendingwallThemesApplication* self) {
  gboolean enabled = g_settings_get_boolean(self->global, "themes");
  if (!enabled) {
    g_application_quit(G_APPLICATION(self));
  }
}

static void activate(MendingwallThemesApplication* self) {
  mendingwall_background_application_activate(MENDINGWALL_BACKGROUND_APPLICATION(self));

  gboolean enabled = g_settings_get_boolean(self->global, "themes");
  if (enabled) {
    /* save or restore settings and possibly watch */
    gchar** schemas = g_key_file_get_string_list(self->config, self->desktop, "GSettings", NULL, NULL);
    for (gchar** schema = schemas; schema && *schema; ++schema) {
      GSettings* settings = g_settings_new(*schema);
      g_ptr_array_add(self->settings, settings);

      if (self->save) {
        save_settings(settings);
      } else if (self->restore) {
        restore_settings(settings);
      }
      if (self->watch) {
        g_signal_connect(settings, "change-event", G_CALLBACK(changed_settings), NULL);
      }
    }
    g_strfreev(schemas);

    /* save or restore config files and possibly watch */
    gchar** paths = g_key_file_get_string_list(self->config, self->desktop, "ConfigFiles", NULL, NULL);
    for (gchar** path = paths; path && *path; ++path) {
      g_autofree char* filename = g_build_filename(g_get_user_config_dir(), *path, NULL);
      GFile* file = g_file_new_for_path(filename);
      g_ptr_array_add(self->files, file);

      if (self->save) {
        save_file(file);
      } else if (self->restore) {
        restore_file(file);
      }
      if (self->watch) {
        GFileMonitor* monitor = g_file_monitor_file(file, G_FILE_MONITOR_NONE, NULL, NULL);
        g_signal_connect_swapped(monitor, "changed", G_CALLBACK(changed_file), file);
        g_ptr_array_add(self->monitors, monitor);
      }
    }
    g_strfreev(paths);
  }

  if (enabled && self->watch) {
    /* quit once feature disabled */
    g_signal_connect_swapped(self->global, "changed::themes", G_CALLBACK(deactivate), self);
  } else {
    /* quit now */
    g_application_quit(G_APPLICATION(self));
  }
}

void mendingwall_themes_application_dispose(GObject* self) {
  MendingwallThemesApplication* app = MENDINGWALL_THEMES_APPLICATION(self);
  g_object_unref(app->global);
  g_key_file_free(app->config);
  g_ptr_array_free(app->settings, TRUE);
  g_ptr_array_free(app->files, TRUE);
  g_ptr_array_free(app->monitors, TRUE);
  G_OBJECT_CLASS(mendingwall_themes_application_parent_class)->dispose(self);
}

void mendingwall_themes_application_finalize(GObject* self) {
  G_OBJECT_CLASS(mendingwall_themes_application_parent_class)->finalize(self);
}

void mendingwall_themes_application_class_init(MendingwallThemesApplicationClass* klass) {
  G_OBJECT_CLASS(klass)->dispose = mendingwall_themes_application_dispose;
  G_OBJECT_CLASS(klass)->finalize = mendingwall_themes_application_finalize;
}

void mendingwall_themes_application_init(MendingwallThemesApplication* self) {
  self->global = g_settings_new("org.indii.mendingwall");
  self->config = g_key_file_new();
  self->settings = g_ptr_array_new_with_free_func(g_object_unref);
  self->files = g_ptr_array_new_with_free_func(g_object_unref);
  self->monitors = g_ptr_array_new_with_free_func(g_object_unref);
  self->desktop = g_getenv("XDG_CURRENT_DESKTOP");
  self->save = FALSE;
  self->restore = FALSE;
  self->watch = FALSE;

  /* check desktop */
  if (!self->desktop) {
    g_printerr("Environment variable XDG_CURRENT_DESKTOP is not set\n");
    exit(1);
  }

  /* load config file */
  if (!g_key_file_load_from_data_dirs(self->config, "mendingwall/themes.conf", NULL, G_KEY_FILE_NONE, NULL)) {
    g_printerr("Cannot find config file mendingwall/themes.conf\n");
    exit(1);
  }

  /* check config file */
  if (!g_key_file_has_group(self->config, self->desktop)) {
    g_printerr("Desktop environment %s is not supported\n", self->desktop);
    exit(1);
  }
}

MendingwallThemesApplication* mendingwall_themes_application_new(void) {
  MendingwallThemesApplication* self = MENDINGWALL_THEMES_APPLICATION(g_object_new(MENDINGWALL_TYPE_THEMES_APPLICATION, "application-id", "org.indii.mendingwall.themes", "flags", G_APPLICATION_DEFAULT_FLAGS, NULL));
  g_application_set_version(G_APPLICATION(self), PACKAGE_VERSION);
  g_signal_connect(self, "handle-local-options", G_CALLBACK(handle_local_options), NULL);
  g_signal_connect(self, "activate", G_CALLBACK(activate), NULL);

  /* command-line options */
  GOptionEntry options[] = {
    { "save", 0, 0, G_OPTION_ARG_NONE, &self->save, "Save theme configuration", NULL },
    { "restore", 0, 0, G_OPTION_ARG_NONE, &self->restore, "Restore theme configuration", NULL },
    { "watch", 0, 0, G_OPTION_ARG_NONE, &self->watch, "Continue to watch for changes and save", NULL },
    G_OPTION_ENTRY_NULL
  };
  g_application_set_option_context_summary(G_APPLICATION(self), "- untangle themes");
  g_application_set_option_context_description(G_APPLICATION(self), "For more information see https://mendingwall.org");
  g_application_add_main_option_entries(G_APPLICATION(self), options);

  return self;
}
