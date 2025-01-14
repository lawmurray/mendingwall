#include <config.h>
#include <mendingwall-menus-application.h>

struct _MendingwallMenusApplication {
  GtkApplication parent_instance;
  GSettings* global;
  GKeyFile* config;
  GPtrArray* dirs;
  GPtrArray* monitors;
  gboolean watch;
};

G_DEFINE_TYPE(MendingwallMenusApplication, mendingwall_menus_application, GTK_TYPE_APPLICATION)

static void process_file(const char* basename, GKeyFile* config) {
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

static void changed_file(GFileMonitor*, GFile* file, GFile*, GFileMonitorEvent event_type, GKeyFile* config) {
  if (event_type == G_FILE_MONITOR_EVENT_CREATED) {
    g_autofree char* basename = g_file_get_basename(file);
    process_file(basename, config);
  }
}

static void deactivate(MendingwallMenusApplication* self) {
  gboolean enabled = g_settings_get_boolean(self->global, "menus");
  if (!enabled) {
    g_application_quit(G_APPLICATION(self));
  }
}

static void query_end(MendingwallMenusApplication* self) {
  g_application_quit(G_APPLICATION(self));
}

static void activate(MendingwallMenusApplication* self) {
  /* what to do */
  gboolean enabled = g_settings_get_boolean(self->global, "menus");
  gboolean watch = self->watch;

  /* process applications */
  if (enabled) {
    gchar** basenames = g_key_file_get_groups(self->config, NULL);
    for (gchar** basename = basenames; basename && *basename; ++basename) {
      process_file(*basename, self->config);
    }
    g_strfreev(basenames);
  }

  /* watch applications */
  if (enabled && watch) {
    g_autoptr(GPtrArray) dirs = g_ptr_array_new_with_free_func(g_object_unref);
    g_autoptr(GPtrArray) monitors = g_ptr_array_new_with_free_func(g_object_unref);
    const gchar* const* paths = g_get_system_data_dirs();
    for (const gchar* const* path = paths; path && *path; ++path) {
      GFile* dir = g_file_new_build_filename(*path, "applications", NULL);
      GFileMonitor* monitor = g_file_monitor_directory(dir, G_FILE_MONITOR_NONE, NULL, NULL);
      g_signal_connect(monitor, "changed", G_CALLBACK(changed_file), self->config);
      g_ptr_array_add(self->dirs, dir);
      g_ptr_array_add(self->monitors, monitor);
    }
  }

  if (enabled && watch) {
    /* quit once feature disabled */
    g_signal_connect_swapped(self->global, "changed::menus", G_CALLBACK(deactivate), self);
    g_application_hold(G_APPLICATION(self));
  } else {
    /* quit now */
    g_application_quit(G_APPLICATION(self));
  }
}

void mendingwall_menus_application_dispose(GObject* self) {
  MendingwallMenusApplication* app = MENDINGWALL_MENUS_APPLICATION(self);
  g_object_unref(app->global);
  g_key_file_free(app->config);
  g_ptr_array_free(app->dirs, TRUE);
  g_ptr_array_free(app->monitors, TRUE);
  G_OBJECT_CLASS(mendingwall_menus_application_parent_class)->dispose(self);
}

void mendingwall_menus_application_finalize(GObject* self) {
  G_OBJECT_CLASS(mendingwall_menus_application_parent_class)->finalize(self);
}

void mendingwall_menus_application_class_init(MendingwallMenusApplicationClass* klass) {
  G_OBJECT_CLASS(klass)->dispose = mendingwall_menus_application_dispose;
  G_OBJECT_CLASS(klass)->finalize = mendingwall_menus_application_finalize;
}

void mendingwall_menus_application_init(MendingwallMenusApplication* self) {
  /* state */
  self->global = g_settings_new("org.indii.mendingwall");
  self->config = g_key_file_new();
  self->dirs = g_ptr_array_new_with_free_func(g_object_unref);
  self->monitors = g_ptr_array_new_with_free_func(g_object_unref);
  self->watch = FALSE;

  /* load config file */
  if (!g_key_file_load_from_data_dirs(self->config, "mendingwall/menus.conf", NULL, G_KEY_FILE_NONE, NULL)) {
    g_printerr("Cannot find config file mendingwall/menus.conf\n");
    exit(1);
  }
}

MendingwallMenusApplication* mendingwall_menus_application_new(void) {
  MendingwallMenusApplication* self = MENDINGWALL_MENUS_APPLICATION(g_object_new(MENDINGWALL_TYPE_MENUS_APPLICATION, "application-id", "org.indii.mendingwall.menus", "flags", G_APPLICATION_DEFAULT_FLAGS, "register-session", TRUE, NULL));
  g_signal_connect(self, "activate", G_CALLBACK(activate), NULL);
  g_signal_connect(self, "query-end", G_CALLBACK(query_end), NULL);

  /* command-line options */
  GOptionEntry options[] = {
    { "watch", 0, 0, G_OPTION_ARG_NONE, &self->watch, "Continue to watch for changes", NULL },
    G_OPTION_ENTRY_NULL
  };
  g_application_set_option_context_summary(G_APPLICATION(self), "- manage application menus");
  g_application_set_option_context_description(G_APPLICATION(self), "For more information see https://mendingwall.org");
  g_application_add_main_option_entries(G_APPLICATION(self), options);

  return self;
}
