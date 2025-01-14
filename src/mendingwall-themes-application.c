#define G_SETTINGS_ENABLE_BACKEND 1

#include <config.h>
#include <mendingwall-themes-application.h>

#include <gio/gsettingsbackend.h>

struct _MendingwallThemesApplication {
  MendingwallBackgroundApplication parent_instance;
  GSettings* global;
  GKeyFile* config;
  GPtrArray* settings;
  GPtrArray* files;
  GPtrArray* monitors;
  const gchar* desktop;
};

G_DEFINE_TYPE(MendingwallThemesApplication, mendingwall_themes_application, MENDINGWALL_TYPE_BACKGROUND_APPLICATION)

static void save_settings(GSettings* from) {
  GSettingsSchema* schema = NULL;
  g_object_get(from, "settings-schema", &schema, NULL);

  const gchar* id = g_settings_schema_get_id(schema);
  const gchar* desktop = g_getenv("XDG_CURRENT_DESKTOP");
  g_autofree gchar* filename = g_strconcat(g_get_user_data_dir(), "/mendingwall/save/", desktop, ".gsettings", NULL);

  g_autoptr(GSettingsBackend) backend = g_keyfile_settings_backend_new(filename, "/", NULL);
  g_autoptr(GSettings) to = g_settings_new_with_backend(id, backend);

  gchar** keys = g_settings_schema_list_keys(schema);
  for (gchar** key = keys; key && *key; ++key) {
    g_autoptr(GVariant) value = g_settings_get_value(from, *key);
    g_settings_set_value(to, *key, value);
  }
  g_strfreev(keys);
}

static void save_file(const gchar* dir, GFile* from) {
  const gchar* desktop = g_getenv("XDG_CURRENT_DESKTOP");
  g_autoptr(GFile) rel_to = g_file_new_for_path(dir);
  g_autofree char* rel = g_file_get_relative_path(rel_to, from);
  g_autoptr(GFile) to = g_file_new_build_filename(g_get_user_data_dir(), "mendingwall", "save", desktop, rel, NULL);
  g_autoptr(GFile) parent = g_file_get_parent(to);
  if (g_file_query_exists(from, NULL)) {
    g_file_make_directory_with_parents(parent, NULL, NULL);
    g_file_copy(from, to, G_FILE_COPY_OVERWRITE|G_FILE_COPY_ALL_METADATA, NULL, NULL, NULL, NULL);
  } else {
    g_file_delete(to, NULL, NULL);
  }
}

static void changed_settings(GSettings* settings) {
  save_settings(settings);
}

static void changed_file(GFileMonitor*, GFile* file, GFile*,
    GFileMonitorEvent event_type, const gchar* dir) {
  if (event_type == G_FILE_MONITOR_EVENT_CREATED ||
      event_type == G_FILE_MONITOR_EVENT_CHANGED ||
      event_type == G_FILE_MONITOR_EVENT_DELETED) {
    save_file(dir, file);
  }
}

static void save_files(MendingwallThemesApplication* self, const gchar* dir, const gchar* key) {
  gchar** paths = g_key_file_get_string_list(self->config, self->desktop, key, NULL, NULL);
  for (gchar** path = paths; path && *path; ++path) {
    g_autofree char* filename = g_build_filename(dir, *path, NULL);
    GFile* file = g_file_new_for_path(filename);
    save_file(dir, file);

    GFileMonitor* monitor = g_file_monitor_file(file, G_FILE_MONITOR_NONE, NULL, NULL);
    g_signal_connect(monitor, "changed", G_CALLBACK(changed_file), (gchar*)dir);
    g_ptr_array_add(self->files, file);
    g_ptr_array_add(self->monitors, monitor);
  }
  g_strfreev(paths);
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
  if (!enabled) {
    /* quit now */
    g_application_quit(G_APPLICATION(self));
  } else {
    /* save and watch settings */
    gchar** schemas = g_key_file_get_string_list(self->config, self->desktop, "GSettings", NULL, NULL);
    for (gchar** schema = schemas; schema && *schema; ++schema) {
      GSettings* setting = g_settings_new(*schema);
      save_settings(setting);
      g_signal_connect(setting, "change-event", G_CALLBACK(changed_settings), NULL);
      g_ptr_array_add(self->settings, setting);
    }
    g_strfreev(schemas);

    /* save and watch files */
    save_files(self, g_get_user_config_dir(), "ConfigFiles");
    save_files(self, g_get_user_state_dir(), "StateFiles");

    /* watch for feature to be disabled, and if so quit */
    g_signal_connect_swapped(self->global, "changed::themes", G_CALLBACK(deactivate), self);

    /* stay running */
    g_application_hold(G_APPLICATION(self));
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
  MendingwallThemesApplication* self = MENDINGWALL_THEMES_APPLICATION(g_object_new(MENDINGWALL_TYPE_THEMES_APPLICATION, "application-id", "org.indii.mendingwall.themes.save", "flags", G_APPLICATION_DEFAULT_FLAGS, NULL));
  g_signal_connect(self, "activate", G_CALLBACK(activate), NULL);
  return self;
}
