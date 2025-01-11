#include <config.h>

#define G_SETTINGS_ENABLE_BACKEND 1

#include <gio/gio.h>
#include <gio/gsettingsbackend.h>
#include <glib.h>
#include <glib-object.h>

void save_settings(GSettings* from) {
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

void restore_settings(GSettings* to) {
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

void save_file(GFile* from) {
  const gchar* desktop = g_getenv("XDG_CURRENT_DESKTOP");
  g_autoptr(GFile) config = g_file_new_for_path(g_get_user_config_dir());
  g_autofree char* rel = g_file_get_relative_path(config, from);
  g_autoptr(GFile) to = g_file_new_build_filename(g_get_user_data_dir(), "mendingwall", "save", desktop, rel, NULL);
  g_autoptr(GFile) dir = g_file_get_parent(to);
  g_file_make_directory_with_parents(dir, NULL, NULL);
  if (g_file_query_exists(from, NULL)) {
    g_file_copy(from, to, G_FILE_COPY_OVERWRITE, NULL, NULL, NULL, NULL);
  } else {
    g_file_delete(to, NULL, NULL);
  }
}

void restore_file(GFile* to) {
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

void changed_settings(GSettings* settings) {
  save_settings(settings);
}

void changed_file(GFileMonitor*, GFile* file, GFile*,
    GFileMonitorEvent event_type) {
  if (event_type == G_FILE_MONITOR_EVENT_CREATED ||
      event_type == G_FILE_MONITOR_EVENT_CHANGED ||
      event_type == G_FILE_MONITOR_EVENT_DELETED) {
    save_file(file);
  }
}

void activate(GApplication *app, GMainLoop* loop) {
  g_main_loop_run(loop);
}

void deactivate(GSettings* settings, gchar* key, GMainLoop* loop) {
  if (g_strcmp0(key, "themes") == 0 && !g_settings_get_boolean(settings, key)) {
    g_main_loop_quit(loop);
  }
}

int main(int argc, char* argv[]) {
  /* command-line options */
  gboolean save = FALSE, restore = FALSE, watch = FALSE;
  GOptionEntry options[] = {
    { "save", 0, 0, G_OPTION_ARG_NONE, &save, "Save current configuration.", NULL },
    { "restore", 0, 0, G_OPTION_ARG_NONE, &restore, "Restore previously-saved configuration.", NULL },
    { "watch", 0, 0, G_OPTION_ARG_NONE, &watch, "Continue to watch for changes and save", NULL },
    G_OPTION_ENTRY_NULL
  };
  GError* error = NULL;
  GOptionContext* context = g_option_context_new ("- protect themes of current desktop environment");
  g_option_context_set_description(context, "For more information see https://mendingwall.org");
  g_option_context_add_main_entries(context, options, GETTEXT_PACKAGE);
  if (!g_option_context_parse(context, &argc, &argv, &error)) {
    g_print("Error: %s\n", error->message);
    exit(1);
  }
  if (!save && !restore) {
    g_print("Error: One of --save or --restore must be used\n");
    exit(1);
  }
  if (save && restore) {
    g_print("Error: Only one of --save or --restore must be used\n");
    exit(1);
  }

  /* settings */
  g_autoptr(GSettings) global = g_settings_new("org.indii.mendingwall");

  /* exit early if not enabled */
  if (!g_settings_get_boolean(global, "themes")) {
    return 0;
  }

  /* config file */
  g_autoptr(GKeyFile) config = g_key_file_new();
  if (!g_key_file_load_from_data_dirs(config, "mendingwall/themes.conf", NULL, G_KEY_FILE_NONE, NULL)) {
    g_print("Error: configuration file mendingwall/themes.conf not found");
    exit(1);
  }

  /* desktop environment */
  const gchar* desktop = g_getenv("XDG_CURRENT_DESKTOP");
  if (!desktop) {
    g_print("Error: Environment variable XDG_CURRENT_DESKTOP is not set");
    exit(1);
  }
  if (!g_key_file_has_group(config, desktop)) {
    g_print("Error: desktop environment %s not supported", desktop);
    exit(1);
  }

  /* save/restore/monitor */
  g_autoptr(GPtrArray) settings = g_ptr_array_new_with_free_func(g_object_unref);
  g_autoptr(GPtrArray) files = g_ptr_array_new_with_free_func(g_object_unref);
  g_autoptr(GPtrArray) monitors = g_ptr_array_new_with_free_func(g_object_unref);

  /* settings */
  gchar** schemas = g_key_file_get_string_list(config, desktop, "GSettings", NULL, NULL);
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
    g_ptr_array_add(settings, setting);
  }
  g_strfreev(schemas);

  /* files */
  gchar** paths = g_key_file_get_string_list(config, desktop, "ConfigFiles", NULL, NULL);
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
      g_ptr_array_add(files, file);
      g_ptr_array_add(monitors, monitor);
    }
  }
  g_strfreev(paths);

  /* main loop */
  if (watch) {
    g_autoptr(GMainLoop) loop = g_main_loop_new(NULL, FALSE);
    GApplication* app = g_application_new("org.indii.mendingwall.themes", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), loop);
    g_signal_connect(global, "changed", G_CALLBACK(deactivate), loop);
    return g_application_run(G_APPLICATION(app), argc, argv);
  } else {
    return 0;
  }
}
