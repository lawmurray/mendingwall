/*
 * Copyright (C) 2025 Lawrence Murray <lawrence@indii.org>
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include <config.h>
#include <utility.h>

#define G_SETTINGS_ENABLE_BACKEND 1
#include <gio/gio.h>
#include <gio/gsettingsbackend.h>

#define MAX_DATA_DIRS 32

static const char* app_config_dir = NULL;
static const char* user_config_dir = NULL;
static const char* app_data_dir = NULL;
static const char* user_data_dir = NULL;
static const char* data_dirs[MAX_DATA_DIRS];
static const char* desktop;
static const char* save_files_dir;
static const char* autostart_dir;
static const char* plasma_workspace_env_dir;
static const char* watch_filename = "org.indii.mendingwall.watch.desktop";
static const char* restore_filename = "org.indii.mendingwall.restore.desktop";
static const char* script_filename = "org.indii.mendingwall.restore.sh";

static GKeyFile* themes_config;
static GKeyFile* menus_config;

static GKeyFile* load_config(const char* path) {
  GKeyFile* config = g_key_file_new();
  if (!g_key_file_load_from_dirs(config, path, get_data_dirs(), NULL,
      G_KEY_FILE_NONE, NULL)) {
    g_printerr("Cannot find config file %s\n", path);
    exit(1);
  }
  return config;
}

void configure_environment(void) {
  /* The purpose of this function is to determine the user config dir, user
   * data dir, and system data dirs of the host and set the above variables
   * accordingly. It is needed because both Flatpak and Snap manipulate
   * environment variables for application isolation, and Mending Wall is not
   * an application that is meant to exist in isolation. It needs access to
   * the user config dir to save and restore desktop environment
   * configuration, and to system and user data directories where .desktop
   * files are installed. */

  #if defined(BUILD_FOR_FLATPAK)
  /* Flatpak overrides XDG_CONFIG_HOME and XDG_DATA_HOME, and unsets HOME. The
   * best way to get the originals back is to temporarily unset them and let
   * g_get_user_config_dir() and g_get_user_data_dir() reconstruct defaults;
   * don't call them until XDG_CONFIG_HOME and XDG_DATA_HOME are unset though,
   * otherwise value is fixed */
  const char* home = g_getenv("HOME");
  app_config_dir = g_getenv("XDG_CONFIG_HOME");
  user_config_dir = g_build_filename(home, ".config", NULL);
  app_data_dir = g_getenv("XDG_DATA_HOME");
  user_data_dir = g_build_filename(home, ".local", "share", NULL);

  /* Hard-code host data directories where applications may be installed. An
   * alternative is to use `flatpak-spawn --host`, but this requires the
   * `--talk-name=org.freedesktop.Flatpak` sandbox permission, which is very
   * broad, and results in a Flathub warning about the app's ability to
   * acquire arbitrary permissions. That alternative approach is used by other
   * apps such as dconf-editor and Refine, however, which offer a good
   * reference if this needs to change. */
  const char* host_system_data_dirs[] = {
    "/var/lib/flatpak/exports/share",
    "/run/host/usr/local/share",
    "/run/host/usr/share",
    "/var/lib/snapd/desktop",
    NULL
  };
  #elif defined(BUILD_FOR_SNAP)
  /* Snap overrides XDG_CONFIG_HOME, XDG_DATA_HOME, and HOME, but sets a new
   * variable SNAP_REAL_HOME to what HOME used to be, which can be used to
   * reconstruct. */
  const char* home = g_getenv("SNAP_REAL_HOME");
  app_config_dir = g_getenv("XDG_CONFIG_HOME");
  user_config_dir = g_build_filename(home, ".config", NULL);
  app_data_dir = g_getenv("XDG_DATA_HOME");
  user_data_dir = g_build_filename(home, ".local", "share", NULL);

  /* Similar to Flatpak, hard code, but locations are different. */
  const char* host_system_data_dirs[] = {
    "/var/lib/snapd/hostfs/var/lib/flatpak/exports/share",
    "/var/lib/snapd/hostfs/usr/local/share",
    "/var/lib/snapd/hostfs/usr/share",
    "/var/lib/snapd/hostfs/var/lib/snapd/desktop",
    NULL
  };
  #else
  /* Everything as normal here. */
  app_config_dir = g_get_user_config_dir();
  user_config_dir = g_get_user_config_dir();
  app_data_dir = g_get_user_data_dir();
  user_data_dir = g_get_user_data_dir();

  const char* host_system_data_dirs[] = {
    NULL
  };
  #endif

  /* construct data_dirs */
  const char** system_data_dirs = (const char**)g_get_system_data_dirs();
  guint i = 0;
  data_dirs[i] = g_strdup(get_app_data_dir());
  ++i;
  for (guint j = 0; i < MAX_DATA_DIRS - 1 && host_system_data_dirs[j]; ++i, ++j) {
    data_dirs[i] = g_strdup(host_system_data_dirs[j]);
  }
  for (guint j = 0; i < MAX_DATA_DIRS - 1 && system_data_dirs[j]; ++i, ++j) {
    data_dirs[i] = g_strdup(system_data_dirs[j]);
  }
  for (; i < MAX_DATA_DIRS; ++i) {
    data_dirs[i] = NULL;
  }

  /* desktop environment */
  desktop = g_getenv("XDG_CURRENT_DESKTOP");
  if (!desktop) {
    g_printerr("Environment variable XDG_CURRENT_DESKTOP is not set\n");
    exit(1);
  }

  /* directories */
  save_files_dir = g_build_filename(app_data_dir, "mendingwall", "save",
      desktop, NULL);
  autostart_dir = g_build_filename(user_config_dir, "autostart", NULL);
  plasma_workspace_env_dir = g_build_filename(user_config_dir,
      "plasma-workspace", "env", NULL);

  /* config files */
  themes_config = load_config("mendingwall/themes.conf");
  menus_config = load_config("mendingwall/menus.conf");
}

const char* get_app_config_dir(void) {
  return app_config_dir;
}

const char* get_user_config_dir(void) {
  return user_config_dir;
}

const char* get_app_data_dir(void) {
  return app_data_dir;
}

const char* get_user_data_dir(void) {
  return user_data_dir;
}

const char** get_data_dirs(void) {
  return data_dirs;
}

const char** get_system_data_dirs(void) {
  return data_dirs + 1;
}

GStrv get_themes_schema_ids(void) {
  const char* group = "Default";
  if (g_key_file_has_group(themes_config, desktop)) {
    group = desktop;
  }
  return g_key_file_get_string_list(themes_config, group, "GSettings", NULL,
      NULL);
}

GStrv get_themes_files(void) {
  const char* group = "Default";
  if (g_key_file_has_group(themes_config, desktop)) {
    group = desktop;
  }
  return g_key_file_get_string_list(themes_config, group, "ConfigFiles", NULL,
      NULL);
}

GStrv get_menus_only_show_in(const char* basename) {
  return g_key_file_get_string_list(menus_config, basename, "OnlyShowIn",
      NULL, NULL);
}

GStrv get_menus_not_show_in(const char* basename) {
  return g_key_file_get_string_list(menus_config, basename, "NotShowIn",
      NULL, NULL);
}

void launch_daemon(GApplication* app) {
  /* ensure that current settings will be visible in new processes */
  g_settings_sync();

  /* launch daemon; fine if already running, new instance will quit */
  #ifdef BUILD_FOR_SNAP
  const gchar* snap = g_getenv("SNAP");
  g_autofree gchar* path = g_build_filename(snap, "usr", "bin", "mendingwalld", NULL);
  const gchar* argv[] = { path, NULL };
  GPid pid;
  g_spawn_async(NULL, (gchar**)argv, NULL, G_SPAWN_DEFAULT, NULL, NULL, &pid, NULL);
  g_spawn_close_pid(pid);
  #else
  g_dbus_connection_call(
      g_application_get_dbus_connection(app),
      "org.indii.mendingwall.watch",
      "/org/indii/mendingwall/watch",
      "org.freedesktop.Application",
      "Activate",
      g_variant_new_parsed("({'': <0>}, )"),
      NULL,
      G_DBUS_CALL_FLAGS_NONE,
      -1,
      NULL,
      NULL,
      NULL
  );
  #endif
}

static void copy(GFile* from_file, GFile* to_file) {
  g_autoptr(GFile) to_dir = g_file_get_parent(to_file);
  g_file_make_directory_with_parents(to_dir, NULL, NULL);
  g_file_copy(from_file, to_file,
      G_FILE_COPY_OVERWRITE|G_FILE_COPY_ALL_METADATA, NULL, NULL, NULL, NULL);
}

static void remove(GFile* file) {
  g_file_delete(file, NULL, NULL);
}

static void install(const char* path, const char* to_dir) {
  g_autoptr(GFile) to_file = g_file_new_build_filename(to_dir, path, NULL);
  foreach (dir, get_data_dirs()) {
    g_autoptr(GFile) file = g_file_new_build_filename(dir, "mendingwall",
         path, NULL);
    if (g_file_query_exists(file, NULL)) {
      copy(file, to_file);
      break;
    }
  }
}

static void uninstall(const char* path, const char* to_dir) {
  g_autoptr(GFile) to_file = g_file_new_build_filename(to_dir, path, NULL);
  g_file_delete_async(to_file, G_PRIORITY_DEFAULT, NULL, NULL, NULL);
}

void install_autostart(void) {
  install(watch_filename, autostart_dir);
  install(restore_filename, autostart_dir);
  install(script_filename, plasma_workspace_env_dir);
}

void uninstall_autostart(void) {
  uninstall(watch_filename, autostart_dir);
  uninstall(restore_filename, autostart_dir);
  uninstall(script_filename, plasma_workspace_env_dir);
}

static GSettingsSchema* get_settings_schema(GSettings* settings) {
  GSettingsSchema* schema = NULL;
  g_object_get(settings, "settings-schema", &schema, NULL);
  return schema;
}

static GSettingsBackend* get_settings_backend(GSettings* settings) {
  g_autofree char* filename = g_strconcat(desktop, ".gsettings", NULL);
  g_autofree char* path = g_build_filename(app_data_dir, "mendingwall",
      "save", filename, NULL);
  return g_keyfile_settings_backend_new(path, "/", NULL);
}

void save_setting(GSettings* settings, gchar* key) {
  g_autoptr(GSettingsSchema) schema = get_settings_schema(settings);
  g_autoptr(GSettingsBackend) backend = get_settings_backend(settings);
  g_autoptr(GSettings) saved = g_settings_new_with_backend(
      g_settings_schema_get_id(schema), backend);
  g_autoptr(GVariant) value = g_settings_get_value(settings, key);
  g_settings_set_value(saved, key, value);
}

static void save_settings(GSettings* settings) {
  g_autoptr(GSettingsSchema) schema = get_settings_schema(settings);
  g_autoptr(GSettingsBackend) backend = get_settings_backend(settings);
  g_autoptr(GSettings) saved = g_settings_new_with_backend(
      g_settings_schema_get_id(schema), backend);

  /* save settings */
  g_settings_delay(saved);
  g_auto(GStrv) keys = g_settings_schema_list_keys(schema);
  foreach(key, keys) {
    g_autoptr(GVariant) value = g_settings_get_value(settings, key);
    g_settings_set_value(saved, key, value);
  }
  g_settings_apply(saved);
}

static void restore_settings(GSettings* settings) {
  g_autoptr(GSettingsSchema) schema = get_settings_schema(settings);
  g_autoptr(GSettingsBackend) backend = get_settings_backend(settings);
  g_autoptr(GSettings) saved = g_settings_new_with_backend(
      g_settings_schema_get_id(schema), backend);

  /* restore settings from file backend; if there are no saved settings this
   * will restore defaults; this is deliberate, it means that when starting a
   * new desktop environment for the first time after running some other
   * desktop environment, existing settings are reset, the user's settings
   * look pristine, and the new desktop environment performs its default
   * initial setup */
  g_settings_delay(settings);
  g_auto(GStrv) keys = g_settings_schema_list_keys(schema);
  foreach(key, keys) {
    g_autoptr(GVariant) value = g_settings_get_value(saved, key);
    g_settings_set_value(settings, key, value);
  }
  g_settings_apply(settings);
  g_settings_sync();
}

void save_file(const char* path) {
  g_autoptr(GFile) file = g_file_new_build_filename(user_config_dir, path, NULL);
  g_autoptr(GFile) save = g_file_new_build_filename(save_files_dir, path, NULL);

  if (g_file_query_exists(file, NULL)) {
    copy(file, save);
  } else {
    /* delete any existing saved file; it does not exist in the current
     * configuration */
    remove(save);
  }
}

static void restore_file(const char* path) {
  g_autoptr(GFile) file = g_file_new_build_filename(user_config_dir, path, NULL);
  g_autoptr(GFile) save = g_file_new_build_filename(save_files_dir, path, NULL);

  if (g_file_query_exists(save, NULL)) {
    copy(save, file);
  } else {
    /* delete file; it does not exist in the desired configuraiton; this
     * happens either because the file is not part of the save, or there is no
     * save; the latter occurs when starting a new desktop environment for the
     * first time after running some other desktop environment, and by
     * deleting all config files, the user's home directory looks pristine,
     * and the new desktop environment performs its default initial setup */
    remove(file);
  }
}

void save_themes(void) {
  /* save settings */
  g_auto(GStrv) schema_ids = get_themes_schema_ids();
  foreach(schema_id, schema_ids) {
    g_autoptr(GSettings) settings = g_settings_new(schema_id);
    save_settings(settings);
  }

  /* save config files */
  g_auto(GStrv) paths = get_themes_files();
  foreach(path, paths) {
    save_file(path);
  }
}

void restore_themes(void) {
  /* restore settings */
  g_auto(GStrv) schema_ids = get_themes_schema_ids();
  foreach(schema_id, schema_ids) {
    g_autoptr(GSettings) settings = g_settings_new(schema_id);
    restore_settings(settings);
  }

  /* restore config files */
  g_auto(GStrv) paths = get_themes_files();
  foreach(path, paths) {
    restore_file(path);
  }
}

static GKeyFile* update_app(const char* basename) {
  /* OnlyShowIn and NotShowIn updates for this application */
  g_auto(GStrv) only_show_in = get_menus_only_show_in(basename);
  g_auto(GStrv) not_show_in = get_menus_not_show_in(basename);

  GKeyFile* app_file = NULL;
  if (only_show_in || not_show_in) {
    app_file = g_key_file_new();
    g_autofree char* app_path = g_build_filename("applications", basename,
        NULL);
    if (g_key_file_load_from_dirs(app_file, app_path, get_system_data_dirs(),
        NULL, G_KEY_FILE_KEEP_COMMENTS|G_KEY_FILE_KEEP_TRANSLATIONS, NULL)) {
      if (only_show_in) {
        g_key_file_set_string_list(app_file, "Desktop Entry", "OnlyShowIn",
            (const gchar* const*)only_show_in, g_strv_length(only_show_in));
      } else {
        g_key_file_remove_key(app_file, "Desktop Entry", "OnlyShowIn", NULL);
      }
      if (not_show_in) {
        g_key_file_set_string_list(app_file, "Desktop Entry", "NotShowIn",
            (const gchar* const*)not_show_in, g_strv_length(not_show_in));
      } else {
        g_key_file_remove_key(app_file, "Desktop Entry", "NotShowIn", NULL);
      }

      /* an extra marker entry ensures that, in the case of untidy_app(),
       * menu entries are not removed if the user has coincidentally made the
       * same changes that Mending Wall would have */
      g_key_file_set_boolean(app_file, "Desktop Entry", "X-MendingWall-Tidy",
          TRUE);
    } else {
      g_key_file_free(app_file);
      app_file = NULL;
    }
  }
  return app_file;
}

static void uninstall_app(const char* basename) {
  /* remove the given desktop entry file in user's applications directory,
   * as long as it contains the X-MendingWall-Tidy entry */
  g_autoptr(GFile) to_file = g_file_new_build_filename(get_user_data_dir(),
      "applications", basename, NULL);
  g_autofree char* to_path = g_file_get_path(to_file);
  g_autoptr(GKeyFile) key_file = g_key_file_new();
  if (g_key_file_load_from_file(key_file, to_path, G_KEY_FILE_NONE, NULL)) {
    if (g_key_file_get_boolean(key_file, "Desktop Entry", "X-MendingWall-Tidy", NULL)) {
      g_file_delete_async(to_file, G_PRIORITY_DEFAULT, NULL, NULL, NULL);
    }
  }
}

void tidy_app(const char* basename) {
  /* load desktop entry file, if it exists */
  g_autoptr(GKeyFile) app_file = update_app(basename);
  if (app_file) {
    /* save to user's applications directory, if it does not already exist
     * there */
    g_autoptr(GFile) to_dir = g_file_new_build_filename(get_user_data_dir(),
        "applications", NULL);
    g_autoptr(GFile) to_file = g_file_resolve_relative_path(to_dir, basename);
    if (!g_file_query_exists(to_file, NULL)) {
      g_file_make_directory_with_parents(to_dir, NULL, NULL);
      g_autofree char* to_path = g_file_get_path(to_file);
      g_key_file_save_to_file(app_file, to_path, NULL);
    }
  } else {
    uninstall_app(basename);
  }
}

void untidy_app(const char* basename) {
  /* load desktop entry file, if it exists */
  g_autoptr(GKeyFile) app_file = update_app(basename);
  if (app_file) {
    /* check if a matching desktop entry file exists in the user's
     * applications directory; if so and its contents match what would be
     * written, delete it, otherwise custom changes have been made so leave
     * it; the match includes the X-MendingWall-Tidy entry */
    g_autoptr(GFile) to_dir = g_file_new_build_filename(get_user_data_dir(),
        "applications", NULL);
    g_autoptr(GFile) to_file = g_file_resolve_relative_path(to_dir, basename);
    if (g_file_query_exists(to_file, NULL)) {
      g_autofree gchar* app_data = g_key_file_to_data(app_file, NULL, NULL);
      g_autofree gchar* to_data = NULL;
      g_file_load_contents(to_file, NULL, &to_data, NULL, NULL, NULL);
      if (g_str_equal(app_data, to_data)) {
        g_file_delete_async(to_file, G_PRIORITY_DEFAULT, NULL, NULL, NULL);
      }
    }
  } else {
    uninstall_app(basename);
  }
}

void tidy_menus(void) {
  g_auto(GStrv) basenames = g_key_file_get_groups(menus_config, NULL);
  foreach(basename, basenames) {
    tidy_app(basename);
  }
}

void untidy_menus(void) {
  g_auto(GStrv) basenames = g_key_file_get_groups(menus_config, NULL);
  foreach(basename, basenames) {
    untidy_app(basename);
  }
}

