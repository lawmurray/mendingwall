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
 * Restore themes.
 */
void restore_themes(void);
