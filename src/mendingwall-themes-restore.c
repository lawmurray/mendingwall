#include <config.h>

#define G_SETTINGS_ENABLE_BACKEND 1

#include <gio/gio.h>
#include <gio/gsettingsbackend.h>

static void restore_settings(GSettings* to) {
  /* schema */
  GSettingsSchema* schema = NULL;
  g_object_get(to, "settings-schema", &schema, NULL);

  /* save file */
  const gchar* id = g_settings_schema_get_id(schema);
  const gchar* desktop = g_getenv("XDG_CURRENT_DESKTOP");
  g_autofree gchar* filename = g_strconcat(g_get_user_data_dir(), "/mendingwall/save/", desktop, ".gsettings", NULL);
  g_autoptr(GFile) file = g_file_new_for_path(filename);

  gchar** keys = g_settings_schema_list_keys(schema);
  if (g_file_query_exists(file, NULL)) {
    /* if the save file exists, restore settings */
    g_autoptr(GSettingsBackend) backend = g_keyfile_settings_backend_new(filename, "/", NULL);
    g_autoptr(GSettings) from = g_settings_new_with_backend(id, backend);
    for (gchar** key = keys; *key; ++key) {
      g_autoptr(GVariant) value = g_settings_get_value(from, *key);
      g_settings_set_value(to, *key, value);
    }
  } else {
    /* if the save file does not exist, restore settings to their defaults;
     * this is deliberate, it means that when starting a new desktop
     * environment for the first time after running some other desktop
     * environment, existing settings are reset, the user's settings look
     * pristine, and the new desktop environment performs its default initial
     * setup */
    for (gchar** key = keys; *key; ++key) {
      g_settings_reset(to, *key);
    }
  }
  g_strfreev(keys);
}

static void restore_file(GFile* to) {
  const gchar* desktop = g_getenv("XDG_CURRENT_DESKTOP");
  g_autoptr(GFile) save = g_file_new_build_filename(g_get_user_data_dir(), "mendingwall", "save", desktop, NULL);
  g_autoptr(GFile) config = g_file_new_for_path(g_get_user_config_dir());
  g_autofree char* rel = g_file_get_relative_path(config, to);
  g_autoptr(GFile) from = g_file_new_build_filename(g_get_user_data_dir(), "mendingwall", "save", desktop, rel, NULL);
  if (g_file_query_exists(from, NULL)) {
    /* restore the file */
    g_file_copy(from, to, G_FILE_COPY_OVERWRITE|G_FILE_COPY_ALL_METADATA, NULL, NULL, NULL, NULL);
  } else {
    /* delete the file; this happens either because the file is not part of
     * the save, or there is no save; the latter occurs when starting a new
     * desktop environment for the first time after running some other desktop
     * environment, and by deleting all config files, the user's home
     * directory looks pristine, and the new desktop environment performs its
     * default initial setup */
    g_file_delete(to, NULL, NULL);
  }
}

int main(int argc, char* argv[]) {
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

  /* restore settings */
  gchar** schemas = g_key_file_get_string_list(config, desktop, "GSettings", NULL, NULL);
  for (gchar** schema = schemas; *schema; ++schema) {
    g_autoptr(GSettings) setting = g_settings_new(*schema);
    restore_settings(setting);
  }
  g_strfreev(schemas);

  /* restore files */
  g_autoptr(GPtrArray) files = g_ptr_array_new_with_free_func(g_object_unref);
  gchar** paths = g_key_file_get_string_list(config, desktop, "ConfigFiles", NULL, NULL);
  for (gchar** path = paths; *path; ++path) {
    g_autofree char* filename = g_build_filename(g_get_user_config_dir(), *path, NULL);
    g_autoptr(GFile) file = g_file_new_for_path(filename);
    restore_file(file);
  }
  g_strfreev(paths);

  return 0;
}