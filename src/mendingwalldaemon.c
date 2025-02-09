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
#include <mendingwalldaemon.h>

#include <libportal/portal.h>

typedef struct {
  XdpPortal* portal;
} MendingwallDaemonPrivate;

G_DEFINE_TYPE_WITH_PRIVATE(MendingwallDaemon, mendingwall_daemon, G_TYPE_APPLICATION)

static void on_session_state_changed(MendingwallDaemon* self, gboolean,
    XdpLoginSessionState* state) {
  MendingwallDaemonPrivate* priv = mendingwall_daemon_get_instance_private(self);
  if (*state == XDP_LOGIN_SESSION_QUERY_END) {
    xdp_portal_session_monitor_query_end_response(priv->portal);
  } else if (*state == XDP_LOGIN_SESSION_ENDING) {
    g_application_quit(G_APPLICATION(self));
  }
}

void mendingwall_daemon_dispose(GObject* self) {
  MendingwallDaemon* app = MENDINGWALL_DAEMON(self);
  MendingwallDaemonPrivate* priv = mendingwall_daemon_get_instance_private(app);

  if (priv->portal) {
    g_clear_object(&priv->portal);
  }

  G_OBJECT_CLASS(mendingwall_daemon_parent_class)->dispose(self);
}

void mendingwall_daemon_finalize(GObject* self) {
  G_OBJECT_CLASS(mendingwall_daemon_parent_class)->finalize(self);
}

void mendingwall_daemon_class_init(MendingwallDaemonClass* klass) {
  G_OBJECT_CLASS(klass)->dispose = mendingwall_daemon_dispose;
  G_OBJECT_CLASS(klass)->finalize = mendingwall_daemon_finalize;
}

void mendingwall_daemon_init(MendingwallDaemon* self) {
  MendingwallDaemonPrivate* priv = mendingwall_daemon_get_instance_private(self);
  priv->portal = NULL;
}

void mendingwall_daemon_hold(MendingwallDaemon* self) {
  MendingwallDaemonPrivate* priv = mendingwall_daemon_get_instance_private(self);

  /* keep running as background process, as application has no main window */
  g_application_hold(G_APPLICATION(self));

  /* register with portal for session end; this is necessary when systemd (or
   * otherwise) is not configured to kill user processes at end of session; it
   * is also used for the monitoring of background apps in GNOME */
  priv->portal = xdp_portal_new();
  g_signal_connect_swapped(priv->portal, "session-state-changed",
      G_CALLBACK(on_session_state_changed), NULL);
}
