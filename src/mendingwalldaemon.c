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

typedef struct {
  GDBusProxy* session_manager;
  GDBusProxy* client_private;
} MendingwallDaemonPrivate;

G_DEFINE_TYPE_WITH_PRIVATE(MendingwallDaemon, mendingwall_daemon, G_TYPE_APPLICATION)

static void on_quit(MendingwallDaemon* self) {
  g_application_quit(G_APPLICATION(self));
}

void mendingwall_daemon_dispose(GObject* self) {
  MendingwallDaemon* app = MENDINGWALL_DAEMON(self);
  MendingwallDaemonPrivate* priv = mendingwall_daemon_get_instance_private(app);

  if (priv->session_manager) {
    g_clear_object(&priv->session_manager);
  }
  if (priv->client_private) {
    g_clear_object(&priv->client_private);
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
  priv->session_manager = NULL;
  priv->client_private = NULL;
}

void mendingwall_daemon_hold(MendingwallDaemon* self) {
  MendingwallDaemonPrivate* priv = mendingwall_daemon_get_instance_private(self);

  /* keep running as background process, as application has no main window */
  g_application_hold(G_APPLICATION(self));

  /* register with org.gnome.SessionManager via dbus to terminate at end of
   * session; this is necessary for Linux distributions that do not have
   * systemd (or otherwise) configured to kill all user processes at end of
   * session */
  const gchar* app_id = g_application_get_application_id(G_APPLICATION(self));
  const gchar* desktop_autostart_id = g_getenv("DESKTOP_AUTOSTART_ID");
  g_autofree const gchar* client_id = g_strdup(desktop_autostart_id ? desktop_autostart_id : "");

  GDBusConnection* dbus = g_application_get_dbus_connection(G_APPLICATION(self));
  if (dbus) {
    priv->session_manager = g_dbus_proxy_new_sync(dbus,
        G_DBUS_PROXY_FLAGS_DO_NOT_AUTO_START |
        G_DBUS_PROXY_FLAGS_DO_NOT_LOAD_PROPERTIES |
        G_DBUS_PROXY_FLAGS_DO_NOT_CONNECT_SIGNALS,
        NULL,
        "org.gnome.SessionManager",
        "/org/gnome/SessionManager",
        "org.gnome.SessionManager",
        NULL,
        NULL);
    if (priv->session_manager) {
      g_autoptr(GVariant) res = g_dbus_proxy_call_sync(
          priv->session_manager,
          "RegisterClient",
          g_variant_new("(ss)", app_id, client_id),
          G_DBUS_CALL_FLAGS_NONE,
          G_MAXINT,
          NULL,
          NULL);
      if (res) {
        g_autofree const gchar* client_path = NULL;
        g_variant_get(res, "(o)", &client_path);
        if (client_path) {
          /* quit on session end */
          priv->client_private = g_dbus_proxy_new_sync(dbus,
              G_DBUS_PROXY_FLAGS_NONE,
              NULL,
              "org.gnome.SessionManager",
              client_path,
              "org.gnome.SessionManager.ClientPrivate",
              NULL,
              NULL);
          if (priv->client_private) {
            g_signal_connect_swapped(
                priv->client_private,
                "g-signal::EndSession",
                G_CALLBACK(on_quit),
                self);
          }
        }
      }
    }
  }
}
