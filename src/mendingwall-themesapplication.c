#define G_SETTINGS_ENABLE_BACKEND 1

#include <config.h>
#include <mendingwall-themesapplication.h>

#include <gio/gio.h>
#include <gio/gsettingsbackend.h>
#include <glib.h>
#include <glib-object.h>

struct _MendingwallThemesApplication {
  GApplication parent_instance;
  GSettings* global;
  GKeyFile* config;
  GMainLoop* loop;
  GPtrArray* settings;
  GPtrArray* files;
  GPtrArray* monitors;
  const gchar* desktop;
  gboolean save;
  gboolean restore;
  gboolean watch;
};

G_DEFINE_TYPE(MendingwallThemesApplication, mendingwall_themes_application, G_TYPE_APPLICATION)

static void save_settings(GSettings* from) {
  GSettingsSchema* schema = NULL;
  g_object_get(from, "settings-schema", &schema, NULL);

  const gchar* id = g_settings_schema_get_id(schema);
  const gchar* desktop = g_getenv("XDG_CURRENT_DESKTOP");
  g_autofree gchar* filename = g_strconcat(g_get_user_data_dir(), "/mendingwall/save/", desktop, ".gsettings", NULL);

  g_autoptr(GSettingsBackend) backend = g_keyfile_settings_backend_new(filename, "/", NULL);
  g_autoptr(GSettings) to = g_settings_new_with_backend(id, backend);

  gchar** keys = g_settings_schema_list_keys(schema);
  for (gchar** key = keys; *key; ++key) {
    g_autoptr(GVariant) value = g_settings_get_value(from, *key);
    g_settings_set_value(to, *key, value);
  }
  g_strfreev(keys);
}

static void restore_settings(GSettings* to) {
  GSettingsSchema* schema = NULL;
  g_object_get(to, "settings-schema", &schema, NULL);

  const gchar* id = g_settings_schema_get_id(schema);
  const gchar* desktop = g_getenv("XDG_CURRENT_DESKTOP");
  g_autofree gchar* filename = g_strconcat(g_get_user_data_dir(), "/mendingwall/save/", desktop, ".gsettings", NULL);

  g_autoptr(GFile) file = g_file_new_for_path(filename);
  if (g_file_query_exists(file, NULL)) {
    g_autoptr(GSettingsBackend) backend = g_keyfile_settings_backend_new(filename, "/", NULL);
    g_autoptr(GSettings) from = g_settings_new_with_backend(id, backend);

    gchar** keys = g_settings_schema_list_keys(schema);
    for (gchar** key = keys; *key; ++key) {
      g_autoptr(GVariant) value = g_settings_get_value(from, *key);
      g_settings_set_value(to, *key, value);
    }
    g_strfreev(keys);
  }
}

static void save_file(GFile* from) {
  const gchar* desktop = g_getenv("XDG_CURRENT_DESKTOP");
  g_autoptr(GFile) config = g_file_new_for_path(g_get_user_config_dir());
  g_autofree char* rel = g_file_get_relative_path(config, from);
  g_autoptr(GFile) to = g_file_new_build_filename(g_get_user_data_dir(), "mendingwall", "save", desktop, rel, NULL);
  g_autoptr(GFile) dir = g_file_get_parent(to);
  if (g_file_query_exists(from, NULL)) {
    g_file_make_directory_with_parents(dir, NULL, NULL);
    g_file_copy(from, to, G_FILE_COPY_OVERWRITE, NULL, NULL, NULL, NULL);
  } else {
    g_file_delete(to, NULL, NULL);
  }
}

static void restore_file(GFile* to) {
  const gchar* desktop = g_getenv("XDG_CURRENT_DESKTOP");
  g_autoptr(GFile) save = g_file_new_build_filename(g_get_user_data_dir(), "mendingwall", "save", desktop, NULL);
  if (g_file_query_exists(save, NULL)) {
    g_autoptr(GFile) config = g_file_new_for_path(g_get_user_config_dir());
    g_autofree char* rel = g_file_get_relative_path(config, to);
    g_autoptr(GFile) from = g_file_new_build_filename(g_get_user_data_dir(), "mendingwall", "save", desktop, rel, NULL);
    if (g_file_query_exists(from, NULL)) {
      g_file_copy(from, to, G_FILE_COPY_OVERWRITE, NULL, NULL, NULL, NULL);
    } else {
      g_file_delete(to, NULL, NULL);
    }
  }
}

static void changed_settings(GSettings* settings) {
  save_settings(settings);
}

static void changed_file(GFileMonitor*, GFile* file, GFile*,
    GFileMonitorEvent event_type) {
  if (event_type == G_FILE_MONITOR_EVENT_CREATED ||
      event_type == G_FILE_MONITOR_EVENT_CHANGED ||
      event_type == G_FILE_MONITOR_EVENT_DELETED) {
    save_file(file);
  }
}

gint handle_local_options(MendingwallThemesApplication* self) {
  if (!self->save && !self->restore) {
    g_print("One of --save or --restore must be used\n");
    return 1;
  }
  if (self->save && self->restore) {
    g_print("Only one of --save or --restore must be used\n");
    return 1;
  }
  return -1;
}

static void deactivate(MendingwallThemesApplication* self) {
  gboolean enabled = g_settings_get_boolean(self->global, "themes");
  if (!enabled) {
    g_main_loop_quit(self->loop);
    g_application_quit(G_APPLICATION(self));
  }
}

static void activate(MendingwallThemesApplication* self) {
  /* what to do */
  gboolean enabled = g_settings_get_boolean(self->global, "themes");
  gboolean save = self->save;
  gboolean restore = self->restore;
  gboolean watch = self->watch;

  /* save/restore/watch settings */
  if (enabled) {
    gchar** schemas = g_key_file_get_string_list(self->config, self->desktop, "GSettings", NULL, NULL);
    for (gchar** schema = schemas; *schema; ++schema) {
      GSettings* setting = g_settings_new(*schema);
      if (save) {
        save_settings(setting);
      } else if (restore) {
        restore_settings(setting);
      }
      if (watch) {
        g_signal_connect(setting, "change-event", G_CALLBACK(changed_settings), NULL);
      }
      g_ptr_array_add(self->settings, setting);
    }
    g_strfreev(schemas);
  }

  /* save/restore/watch files */
  if (enabled) {
    gchar** paths = g_key_file_get_string_list(self->config, self->desktop, "ConfigFiles", NULL, NULL);
    for (gchar** path = paths; *path; ++path) {
      g_autofree char* filename = g_build_filename(g_get_user_config_dir(), *path, NULL);
      GFile* file = g_file_new_for_path(filename);
      if (save) {
        save_file(file);
      } else if (restore) {
        restore_file(file);
      }
      if (watch) {
        GFileMonitor* monitor = g_file_monitor_file(file, G_FILE_MONITOR_NONE, NULL, NULL);
        g_signal_connect(monitor, "changed", G_CALLBACK(changed_file), NULL);
        g_ptr_array_add(self->files, file);
        g_ptr_array_add(self->monitors, monitor);
      }
    }
    g_strfreev(paths);
  }

  if (enabled && watch) {
    /* quit once feature disabled */
    g_signal_connect_swapped(self->global, "changed::themes", G_CALLBACK(deactivate), self);
    g_main_loop_run(self->loop);
  } else {
    /* quit now */
    g_application_quit(G_APPLICATION(self));
  }
}

void mendingwall_themes_application_class_init(MendingwallThemesApplicationClass* klass) {
  //
}

void mendingwall_themes_application_init(MendingwallThemesApplication* self) {
  self->global = g_settings_new("org.indii.mendingwall");
  self->config = g_key_file_new();
  self->loop = g_main_loop_new(NULL, FALSE);
  self->settings = g_ptr_array_new_with_free_func(g_object_unref);
  self->files = g_ptr_array_new_with_free_func(g_object_unref);
  self->monitors = g_ptr_array_new_with_free_func(g_object_unref);
  self->desktop = g_getenv("XDG_CURRENT_DESKTOP");
  self->save = FALSE;
  self->restore = FALSE;
  self->watch = FALSE;

  /* check desktop */
  if (!self->desktop) {
    g_print("Environment variable XDG_CURRENT_DESKTOP is not set");
    exit(1);
  }

  /* load config file */
  if (!g_key_file_load_from_data_dirs(self->config, "mendingwall/themes.conf", NULL, G_KEY_FILE_NONE, NULL)) {
    g_print("mendingwall/themes.conf file not found");
    exit(1);
  }

  /* check config file */
  if (!g_key_file_has_group(self->config, self->desktop)) {
    g_print("Desktop environment %s not supported", self->desktop);
    exit(1);
  }
}

void mendingwall_themes_application_dispose(GObject* self) {
  MendingwallThemesApplication* app = MENDINGWALL_THEMES_APPLICATION(self);
  g_free(app->global);
  g_key_file_free(app->config);
  g_free(app->loop);
  g_ptr_array_free(app->settings, TRUE);
  g_ptr_array_free(app->files, TRUE);
  g_ptr_array_free(app->monitors, TRUE);
  G_OBJECT_CLASS(mendingwall_themes_application_parent_class)->dispose(self);
}

void mendingwall_themes_application_finalize(GObject* self) {
  G_OBJECT_CLASS(mendingwall_themes_application_parent_class)->finalize(self);
}

MendingwallThemesApplication* mendingwall_themes_application_new(void) {
  MendingwallThemesApplication* self = MENDINGWALL_THEMES_APPLICATION(g_object_new(MENDINGWALL_TYPE_THEMES_APPLICATION, NULL));
  g_signal_connect(self, "activate", G_CALLBACK(activate), NULL);
  g_signal_connect(self, "handle-local-options", G_CALLBACK(handle_local_options), NULL);

  /* command-line options */
  GOptionEntry options[] = {
    { "save", 0, 0, G_OPTION_ARG_NONE, &self->save, "Save current configuration.", NULL },
    { "restore", 0, 0, G_OPTION_ARG_NONE, &self->restore, "Restore previously-saved configuration.", NULL },
    { "watch", 0, 0, G_OPTION_ARG_NONE, &self->watch, "Continue to watch for changes and save", NULL },
    G_OPTION_ENTRY_NULL
  };
  g_application_set_option_context_summary(G_APPLICATION(self), "- manage application menus");
  g_application_set_option_context_description(G_APPLICATION(self), "For more information see https://mendingwall.org");
  g_application_add_main_option_entries(G_APPLICATION(self), options);

  return self;
}
