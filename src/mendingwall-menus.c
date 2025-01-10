#include <config.h>

#include <gio/gio.h>
#include <glib.h>
#include <glib-object.h>

void process_file(GFile* from) {
  /* file name (only) of the desktop entry */
  g_autofree char* entry = g_file_get_basename(from);

  /* config for this desktop entry, if any */
  g_autoptr(GKeyFile) config = g_key_file_new();
  g_key_file_load_from_data_dirs(config, "mendingwall/menus.conf", NULL, G_KEY_FILE_NONE, NULL);
  gchar** only_show_in = g_key_file_get_string_list(config, entry, "OnlyShowIn", NULL, NULL);
  gchar** not_show_in = g_key_file_get_string_list(config, entry, "NotShowIn", NULL, NULL);

  if (only_show_in || not_show_in) {
    g_printerr("a1\n");
    /* copy the desktop entry into user's home directory, with changes, if it
     * does not already exist there */
    g_autoptr(GKeyFile) app = g_key_file_new();
    g_autofree gchar* from_path = g_file_get_path(from);
    if (g_key_file_load_from_file(app, from_path, G_KEY_FILE_KEEP_COMMENTS|G_KEY_FILE_KEEP_TRANSLATIONS, NULL)) {
      if (only_show_in) {
        g_key_file_set_string_list(app, "Desktop Entry", "OnlyShowIn", (const gchar* const*)only_show_in, g_strv_length(only_show_in));
      }
      if (not_show_in) {
        g_key_file_set_string_list(app, "Desktop Entry", "NotShowIn", (const gchar* const*)not_show_in, g_strv_length(not_show_in));
      }

      g_autoptr(GFile) to = g_file_new_build_filename(g_get_user_data_dir(), "applications", entry, NULL);
      if (!g_file_query_exists(to, NULL)) {
        g_autofree gchar* to_path = g_file_get_path(to);
        g_key_file_save_to_file(app, to_path, NULL);
      }
    }
  }

  /* clean up */
  if (only_show_in) {
    g_strfreev(only_show_in);
  }
  if (not_show_in) {
    g_strfreev(not_show_in);
  }
}

void changed_file(GFileMonitor*, GFile* file, GFile*,
    GFileMonitorEvent event_type) {
  if (event_type == G_FILE_MONITOR_EVENT_CREATED) {
    process_file(file);
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
