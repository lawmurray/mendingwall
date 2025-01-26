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
  g_clear_object(&priv->session_manager);
  g_clear_object(&priv->client_private);
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

void mendingwall_daemon_on_startup(MendingwallDaemon* self) {
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
      g_autoptr(GVariant) params = g_variant_new("(ss)", app_id, client_id);
      g_autoptr(GVariant) res = g_dbus_proxy_call_sync(
          priv->session_manager,
          "RegisterClient",
          params,
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
