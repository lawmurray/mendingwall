#include <config.h>

#include <gio/gio.h>
#include <glib.h>
#include <glib-object.h>

void save_settings(GSettings* settings) {
  GValue path = G_VALUE_INIT;
  g_value_init(&path, G_TYPE_STRING);
  g_object_get_property(G_OBJECT(settings), "path", &path);
  g_printerr("save: %s\n", g_value_get_string(&path));
  g_value_unset(&path);
}

void restore_settings(GSettings* settings) {
  GValue path = G_VALUE_INIT;
  g_value_init(&path, G_TYPE_STRING);
  g_object_get_property(G_OBJECT(settings), "path", &path);
  g_printerr("restore: %s\n", g_value_get_string(&path));
  g_value_unset(&path);
}

void forget_settings(GSettings* settings) {
  GValue path = G_VALUE_INIT;
  g_value_init(&path, G_TYPE_STRING);
  g_object_get_property(G_OBJECT(settings), "path", &path);
  g_printerr("forget: %s\n", g_value_get_string(&path));
  g_value_unset(&path);
}

void save_file(GFile* file) {
  g_printerr("save: %s\n", g_file_get_path(file));
}

void restore_file(GFile* file) {
  g_printerr("restore: %s\n", g_file_get_path(file));
}

void forget_file(GFile* file) {
  g_printerr("forget: %s\n", g_file_get_path(file));
}

void changed_settings(GSettings* settings, gchar* key) {
  save_settings(settings);
}

void changed_file(GFileMonitor* self, GFile* file, GFile* other_file,
    GFileMonitorEvent event_type, gpointer user_data) {
  if (event_type == G_FILE_MONITOR_EVENT_CREATED ||
      event_type == G_FILE_MONITOR_EVENT_CHANGED) {
    save_file(file);
  } else if (event_type == G_FILE_MONITOR_EVENT_DELETED) {
    forget_file(file);
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
  /* settings */
  g_autoptr(GSettings) global = g_settings_new("org.indii.mendingwall");

  /* check if feature is enabled */
  if (!g_settings_get_boolean(global, "themes")) {
    return 0;
  }

  /* determine desktop environment */
  const gchar* desktop = g_getenv("XDG_CURRENT_DESKTOP");
  if (!desktop) {
    g_error("environment variable XDG_CURRENT_DESKTOP is not set");
  }

  /* process config */
  g_autoptr(GKeyFile) config = g_key_file_new();
  if (!g_key_file_load_from_data_dirs(config, "mendingwall/themes.conf", NULL, G_KEY_FILE_NONE, NULL)) {
    g_error("mendingwall/themes.conf file not found");
  }
  if (!g_key_file_has_group(config, desktop)) {
    g_error("desktop environment %s not found in mendingwall/themes.conf", desktop);
  }

  /* monitor */
  GPtrArray* settings = g_ptr_array_new_with_free_func(g_object_unref);
  GPtrArray* files = g_ptr_array_new_with_free_func(g_object_unref);
  GPtrArray* monitors = g_ptr_array_new_with_free_func(g_object_unref);
  gsize len = 0;

  /* monitor settings schemas */
  gchar** schemas = g_key_file_get_string_list(config, desktop, "GSettings", &len, NULL);
  for (guint i = 0; i < len; ++i) {
    g_printerr("watching: %s\n", schemas[i]);
    GSettings* setting = g_settings_new(schemas[i]);
    g_signal_connect(setting, "changed", G_CALLBACK(changed_settings), NULL);
    g_ptr_array_add(settings, setting);
  }
  g_strfreev(schemas);

  /* monitor files */
  gchar** paths = g_key_file_get_string_list(config, desktop, "ConfigFiles", &len, NULL);
  for (guint i = 0; i < len; ++i) {
    char* filename = g_build_filename(g_get_user_config_dir(), paths[i], NULL);
    GFile* file = g_file_new_for_path(filename);
    g_printerr("watching: %s\n", filename);
    g_free(filename);
    GFileMonitor* monitor = g_file_monitor_file(file, G_FILE_MONITOR_NONE, NULL, NULL);
    g_signal_connect(monitor, "changed", G_CALLBACK(changed_file), NULL);
    g_ptr_array_add(files, file);
    g_ptr_array_add(monitors, monitor);
  }
  g_strfreev(paths);

  /* start main loop */
  g_autoptr(GMainLoop) loop = g_main_loop_new(NULL, FALSE);
  GApplication* app = g_application_new("org.indii.mendingwall-themes", G_APPLICATION_DEFAULT_FLAGS);
  g_signal_connect(app, "activate", G_CALLBACK(activate), loop);
  g_signal_connect(global, "changed", G_CALLBACK(deactivate), loop);
  int result = g_application_run(G_APPLICATION(app), argc, argv);

  /* clean up */
  g_ptr_array_unref(settings);
  g_ptr_array_unref(files);
  g_ptr_array_unref(monitors);

  return result;
}
