#include <config.h>

#include <gio/gio.h>
#include <glib.h>
#include <glib-object.h>

void changed_file(GFileMonitor*, GFile* file, GFile*,
    GFileMonitorEvent event_type) {
  if (event_type == G_FILE_MONITOR_EVENT_CREATED) {
    g_printerr("created: %s\n", g_file_get_path(file));
  }
}

void activate(GApplication *app, GMainLoop* loop) {
  g_main_loop_run(loop);
}

void deactivate(GSettings* settings, gchar* key, GMainLoop* loop) {
  if (g_strcmp0(key, "menus") == 0 && !g_settings_get_boolean(settings, key)) {
    g_main_loop_quit(loop);
  }
}

int main(int argc, char* argv[]) {
  /* settings */
  g_autoptr(GSettings) global = g_settings_new("org.indii.mendingwall");

  /* check if feature is enabled */
  if (!g_settings_get_boolean(global, "menus")) {
    return 0;
  }

  /* process config */
  g_autoptr(GKeyFile) config = g_key_file_new();
  if (!g_key_file_load_from_data_dirs(config, "mendingwall/menus.conf", NULL, G_KEY_FILE_NONE, NULL)) {
    g_error("mendingwall/menus.conf file not found");
  }

  /* monitor */
  g_autoptr(GPtrArray) dirs = g_ptr_array_new_with_free_func(g_object_unref);
  g_autoptr(GPtrArray) monitors = g_ptr_array_new_with_free_func(g_object_unref);

  /* monitor data directories */
  const gchar* const* paths = g_get_system_data_dirs();
  for (const gchar* const* path = paths; *path; ++path) {
    GFile* dir = g_file_new_build_filename(*path, "applications", NULL);
    GFileMonitor* monitor = g_file_monitor_directory(dir, G_FILE_MONITOR_NONE, NULL, NULL);
    g_signal_connect(monitor, "changed", G_CALLBACK(changed_file), NULL);
    g_ptr_array_add(dirs, dir);
    g_ptr_array_add(monitors, monitor);
  }

  /* start main loop */
  g_autoptr(GMainLoop) loop = g_main_loop_new(NULL, FALSE);
  GApplication* app = g_application_new("org.indii.mendingwall-menus", G_APPLICATION_DEFAULT_FLAGS);
  g_signal_connect(app, "activate", G_CALLBACK(activate), loop);
  g_signal_connect(global, "changed", G_CALLBACK(deactivate), loop);
  return g_application_run(G_APPLICATION(app), argc, argv);
}
