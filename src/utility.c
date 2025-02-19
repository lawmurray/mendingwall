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

static GFile* app_config_dir = NULL;
static GFile* user_config_dir = NULL;
static const char* app_data_dir = NULL;
static const char* user_data_dir = NULL;
static const char* data_dirs[64];

static const char* desktop;
static const char* save_settings_path;
static GFile* save_files_dir;

static GFile* autostart_dir;
static GFile* kde_env_dir;
static const char* watch_path;
static const char* restore_path;
static const char* kde_path;

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
  app_config_dir = g_file_new_for_path(g_getenv("XDG_CONFIG_HOME"));
  user_config_dir = g_file_new_build_filename(home, ".config", NULL);
  app_data_dir = g_strdup(g_getenv("XDG_DATA_HOME"));
  user_data_dir = g_strconcat(home, "/.local/share", NULL);

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
  app_config_dir = g_file_new_for_path(g_getenv("XDG_CONFIG_HOME"));
  user_config_dir = g_file_new_build_filename(home, ".config", NULL);
  app_data_dir = g_strdup(g_getenv("XDG_DATA_HOME"));
  user_data_dir = g_strconcat(home, "/.local/share", NULL);

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
  app_config_dir = g_file_new_for_path(g_get_user_config_dir());
  user_config_dir = g_file_new_for_path(g_get_user_config_dir());
  app_data_dir = g_strdup(g_get_user_data_dir());
  user_data_dir = g_strdup(g_get_user_data_dir());

  const char* host_system_data_dirs[] = {
    NULL
  };
  #endif

  /* construct data_dirs */
  const char** system_data_dirs = (const char**)g_get_system_data_dirs();
  guint i = 0;
  data_dirs[i] = g_strdup(get_app_data_dir());
  ++i;
  for (guint j = 0; i < 63 && host_system_data_dirs[j]; ++i, ++j) {
    data_dirs[i] = g_strdup(host_system_data_dirs[j]);
  }
  for (guint j = 0; i < 63 && system_data_dirs[j]; ++i, ++j) {
    data_dirs[i] = g_strdup(system_data_dirs[j]);
  }
  for (; i < 64; ++i) {
    data_dirs[i] = NULL;
  }

  /* construct save paths */
  desktop = g_getenv("XDG_CURRENT_DESKTOP");
  if (!desktop) {
    g_printerr("Environment variable XDG_CURRENT_DESKTOP is not set\n");
    exit(1);
  }
  g_autofree char* filename = g_strconcat(desktop, ".gsettings", NULL);
  save_settings_path = g_build_filename(app_data_dir, "mendingwall", "save",
      filename, NULL);
  save_files_dir = g_file_new_build_filename(app_data_dir, "mendingwall",
      "save", desktop, NULL);

  /* autostart paths */
  autostart_dir = g_file_resolve_relative_path(user_config_dir,
      "autostart");
  kde_env_dir = g_file_resolve_relative_path(user_config_dir,
      "plasma-workspace/env");

  watch_path = "org.indii.mendingwall.watch.desktop";
  restore_path = "org.indii.mendingwall.restore.desktop";
  kde_path = "org.indii.mendingwall.restore.sh";
}

GFile* get_app_config_dir(void) {
  return app_config_dir;
}

GFile* get_user_config_dir(void) {
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

static const char* get_desktop(void) {
  return desktop;
}

static const char* get_save_settings_path(void) {
  return save_settings_path;
}

static GFile* get_save_files_dir(void) {
  return save_files_dir;
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
  g_file_copy_async(from_file, to_file,
      G_FILE_COPY_OVERWRITE|G_FILE_COPY_ALL_METADATA, G_PRIORITY_DEFAULT,
      NULL, NULL, NULL, NULL, NULL);
}

static void remove(GFile* file) {
  g_file_delete_async(file, G_PRIORITY_DEFAULT, NULL, NULL, NULL);
}

static void install(const char* path, GFile* to_dir) {
  g_autoptr(GFile) to_file = g_file_resolve_relative_path(to_dir, path);
  foreach (dir, get_data_dirs()) {
    g_autoptr(GFile) file = g_file_new_build_filename(dir, "mendingwall",
         path, NULL);
    if (g_file_query_exists(file, NULL)) {
      copy(file, to_file);
      break;
    }
  }
}

static void uninstall(const char* path, GFile* to_dir) {
  g_autoptr(GFile) to_file = g_file_resolve_relative_path(to_dir, path);
  g_file_delete_async(to_file, G_PRIORITY_DEFAULT, NULL, NULL, NULL);
}

void install_autostart(void) {
  install(watch_path, autostart_dir);
  install(restore_path, autostart_dir);
  install(kde_path, kde_env_dir);
}

void uninstall_autostart(void) {
  uninstall(watch_path, autostart_dir);
  uninstall(restore_path, autostart_dir);
  uninstall(kde_path, kde_env_dir);
}

static GSettingsSchema* get_settings_schema(GSettings* settings) {
  GSettingsSchema* schema = NULL;
  g_object_get(settings, "settings-schema", &schema, NULL);
  return schema;
}

static GSettingsBackend* get_settings_backend(GSettings* settings) {
  return g_keyfile_settings_backend_new(get_save_settings_path(), "/", NULL);
}

void save_setting(GSettings* settings, gchar* key) {
  g_autoptr(GSettingsSchema) schema = get_settings_schema(settings);
  g_autoptr(GSettingsBackend) backend = get_settings_backend(settings);
  g_autoptr(GSettings) saved = g_settings_new_with_backend(
      g_settings_schema_get_id(schema), backend);
  g_autoptr(GVariant) value = g_settings_get_value(settings, key);
  g_settings_set_value(saved, key, value);
}

void save_settings(GSettings* settings) {
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
}

void save_file(const char* path) {
  g_autoptr(GFile) file = g_file_resolve_relative_path(get_user_config_dir(),
      path);
  g_autoptr(GFile) save = g_file_resolve_relative_path(get_save_files_dir(),
      path);

  if (g_file_query_exists(file, NULL)) {
    copy(file, save);
  } else {
    /* delete any existing saved file; it does not exist in the current
     * configuration */
    remove(save);
  }
}

static void restore_file(const char* path) {
  g_autoptr(GFile) file = g_file_resolve_relative_path(get_user_config_dir(),
      path);
  g_autoptr(GFile) save = g_file_resolve_relative_path(get_save_files_dir(),
      path);

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

void restore_themes(void) {
  /* load themes config file */
  g_autoptr(GKeyFile) themes_config = g_key_file_new();
  if (!g_key_file_load_from_data_dirs(themes_config,
      "mendingwall/themes.conf", NULL, G_KEY_FILE_NONE, NULL)) {
    g_printerr("Cannot find config file mendingwall/themes.conf\n");
    exit(1);
  }
  if (!g_key_file_has_group(themes_config, get_desktop())) {
    g_printerr("Desktop environment %s is not supported\n", get_desktop());
    exit(1);
  }

  /* restore settings */
  g_auto(GStrv) schema_ids = g_key_file_get_string_list(themes_config,
      get_desktop(), "GSettings", NULL, NULL);
  foreach(schema_id, schema_ids) {
    g_autoptr(GSettings) settings = g_settings_new(schema_id);
    restore_settings(settings);
  }

  /* restore config files */
  g_auto(GStrv) paths = g_key_file_get_string_list(themes_config,
      get_desktop(), "ConfigFiles", NULL, NULL);
  foreach(path, paths) {
    restore_file(path);
  }
}
