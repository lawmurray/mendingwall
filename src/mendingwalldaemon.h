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

#include <config.h>

#include <gio/gio.h>

G_BEGIN_DECLS

#define MENDINGWALL_TYPE_DAEMON mendingwall_daemon_get_type()
G_DECLARE_DERIVABLE_TYPE(MendingwallDaemon, mendingwall_daemon, MENDINGWALL, DAEMON, GApplication)

struct _MendingwallDaemonClass {
  GApplicationClass parent_class;
  gpointer padding[12];
};

void mendingwall_daemon_hold(MendingwallDaemon* self);

G_END_DECLS
