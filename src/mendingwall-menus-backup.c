#include <config.h>

#include <gio/gio.h>
#include <glib.h>
#include <glib-object.h>

struct State {
  GPtrArray* dirs;
  GPtrArray* monitors;
};

void process_file(const char* basename, GKeyFile* config) {
  /* config for this desktop entry */
  gchar** only_show_in = g_key_file_get_string_list(config, basename, "OnlyShowIn", NULL, NULL);
  gchar** not_show_in = g_key_file_get_string_list(config, basename, "NotShowIn", NULL, NULL);

  if (only_show_in || not_show_in) {
    /* copy the desktop entry into user's home directory, with changes, if it
     * does not already exist there */
    g_autoptr(GKeyFile) entry = g_key_file_new();
    g_autofree char* rel = g_build_filename("applications", basename, NULL);
    if (g_key_file_load_from_data_dirs(entry, rel, NULL, G_KEY_FILE_KEEP_COMMENTS|G_KEY_FILE_KEEP_TRANSLATIONS, NULL)) {
      if (only_show_in) {
        g_key_file_set_string_list(entry, "Desktop Entry", "OnlyShowIn", (const gchar* const*)only_show_in, g_strv_length(only_show_in));
      }
      if (not_show_in) {
        g_key_file_set_string_list(entry, "Desktop Entry", "NotShowIn", (const gchar* const*)not_show_in, g_strv_length(not_show_in));
      }
      g_autoptr(GFile) to = g_file_new_build_filename(g_get_user_data_dir(), "applications", basename, NULL);
      if (!g_file_query_exists(to, NULL)) {
        g_autofree gchar* to_path = g_file_get_path(to);
        g_key_file_save_to_file(entry, to_path, NULL);
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

void changed_file(GFileMonitor*, GFile* file, GFile*, GFileMonitorEvent event_type, GKeyFile* config) {
  if (event_type == G_FILE_MONITOR_EVENT_CREATED) {
    g_autofree char* basename = g_file_get_basename(file);
    process_file(basename, config);
  }
}

void activate(GApplication* app, State* state) {
  /* exit early if not enabled */
  if (!g_settings_get_boolean(state->global, "menus")) {
    g_application_quit(app);
  }

  /* read config */
  if (!g_key_file_load_from_data_dirs(state->config, "mendingwall/menus.conf", NULL, G_KEY_FILE_NONE, NULL)) {
    g_error("mendingwall/menus.conf file not found");
  }

  /* process applications */
  gchar** basenames = g_key_file_get_groups(state->config, NULL);
  for (gchar** basename = basenames; *basename; ++basename) {
    process_file(*basename, state->config);
  }
  g_strfreev(basenames);
}

gint handle_local_options(GApplication* app, GVariantDict* options, State* state) {
  if (watch) {
    /* watch applications */
    const gchar* const* paths = g_get_system_data_dirs();
    for (const gchar* const* path = paths; *path; ++path) {
      GFile* dir = g_file_new_build_filename(*path, "applications", NULL);
      GFileMonitor* monitor = g_file_monitor_directory(dir, G_FILE_MONITOR_NONE, NULL, NULL);
      g_signal_connect(monitor, "changed", G_CALLBACK(changed_file), config);
      g_ptr_array_add(state->dirs, dir);
      g_ptr_array_add(state->monitors, monitor);
    }
  }
}

void deactivate(GSettings* settings, gchar* key, GApplication* app) {
  if (g_strcmp0(key, "menus") == 0 && !g_settings_get_boolean(settings, key)) {
    g_application_quit(app);
  }
}

int main(int argc, char* argv[]) {
  GApplication* app = g_application_new("org.indii.mendingwall.menus", G_APPLICATION_DEFAULT_FLAGS);

  /* command-line options */
  gboolean watch = FALSE;
  GOptionEntry options[] = {
    { "watch", 0, 0, G_OPTION_ARG_NONE, &watch, "Continue to watch for changes", NULL },
    G_OPTION_ENTRY_NULL
  };
  g_application_set_option_context_summary(app, "- manage application menus");
  g_application_set_option_context_description(app, "For more information see https://mendingwall.org");
  g_application_add_main_option_entries(context, options, GETTEXT_PACKAGE);

  /* state */
  g_autoptr(GSettings) global = g_settings_new("org.indii.mendingwall");
  g_autoptr(GPtrArray) dirs = g_ptr_array_new_with_free_func(g_object_unref);
  g_autoptr(GPtrArray) monitors = g_ptr_array_new_with_free_func(g_object_unref);
  g_autoptr(GKeyFile) config = g_key_file_new();
  State state = { global, config, dirs, monitors };

  /* signals */
  g_signal_connect(app, "activate", G_CALLBACK(activate), &state);
  g_signal_connect(app, "handle-local-options", G_CALLBACK(activate), &state);
  g_signal_connect(global, "changed", G_CALLBACK(deactivate), app);

  return g_application_run(G_APPLICATION(app), argc, argv);
}
