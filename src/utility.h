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
#pragma once

#include <gio/gio.h>

#define foreach_with_iterator(value, values, iterator) \
    typeof(values) iterator = values; \
    for (typeof(*iterator) value = *iterator; value; value = *++iterator)
#define foreach_iterator(line) \
    iter_ ## line ## _
#define foreach_with_line(value, values, line) \
    foreach_with_iterator(value, values, foreach_iterator(line))

/**
 * A tidier way to loop over null-terminated arrays of pointers.
 * 
 * ```c
 * foreach(value, values) {
 *   //
 * }
 * ```
 */
#define foreach(value, values) \
    foreach_with_line(value, values, __LINE__)

/**
 * Configure the environment.
 *
 * This sets up some internal variables for convenient access to standard host
 * directories (e.g. ~/.config, ~/.local/share) that are otherwise confused by
 * Flatpak and Snap manipulations of environment variables.
 */
void configure_environment(void);

/**
 * Get app config directory.
 *
 * This is given by the environment variable XDG_CONFIG_HOME or GLib
 * g_get_user_config_dir(), and is subject to Flatpak and Snap manipulations.
 * It can be interpreted as an app-specific config directory that only Mending
 * Wall has access to.
 */
GFile* get_app_config_dir(void);

/**
 * Get user config directory.
 * 
 * This is always the directory ~/.config, regardless of Flatpak and Snap
 * manipulations. It can be interpreted as a user-specific config directory
 * used by multiple applications and desktop environments.
 */
GFile* get_user_config_dir(void);

/**
 * Get app data directory.
 *
 * This is given by the environment variable XDG_DATA_HOME or GLib
 * g_get_user_data_dir(), subject to Flatpak and Snap manipulations. It can be
 * interpreted as an app-specific config directory that only Mending Wall has
 * access to.
 */
const char* get_app_data_dir(void);

/**
 * Get user data directory.
 * 
 * This is always the directory ~/.local/share, regardless of Flatpak and
 * Snap manipulations. It can be interpted as a user-specific data directory
 * used by multiple applications and desktop environments.
 */
const char* get_user_data_dir(void);

/**
 * Get data directories.
 *
 * This is get_app_data_dir() and get_system_data_dirs() appended.
 */
const char** get_data_dirs(void);

/**
 * Get system data directories.
 *
 * This is given by the environment variable XDG_DATA_DIRS or GLib
 * g_get_system_data_dirs() but is what would be obtained on host, then
 * appending the manipulated values from Flatpak and Snap.
 */
const char** get_system_data_dirs(void);

/**
 * Launch the background process.
 */
void launch_daemon(GApplication* app);

/**
 * Install autostart files.
 */
void install_autostart(void);

/**
 * Install autostart files.
 */
void uninstall_autostart(void);

/**
 * Save settings.
 */
void save_settings(GSettings* settings);

/**
 * Restore themes.
 */
void restore_themes(void);
