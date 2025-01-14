#include <config.h>
#include <mendingwall-background-application.h>

typedef struct {
  GDBusProxy* session;
  GDBusProxy* client_private;
} MendingwallBackgroundApplicationPrivate;

G_DEFINE_TYPE_WITH_PRIVATE(MendingwallBackgroundApplication, mendingwall_background_application, G_TYPE_APPLICATION)

static void quit(MendingwallBackgroundApplication* self) {
  g_application_quit(G_APPLICATION(self));
}

void mendingwall_background_application_dispose(GObject* self) {
  MendingwallBackgroundApplication* app = MENDINGWALL_BACKGROUND_APPLICATION(self);
  MendingwallBackgroundApplicationPrivate* priv = mendingwall_background_application_get_instance_private(app);
  g_clear_object(&priv->session);
  g_clear_object(&priv->client_private);
  G_OBJECT_CLASS(mendingwall_background_application_parent_class)->dispose(self);
}

void mendingwall_background_application_finalize(GObject* self) {
  G_OBJECT_CLASS(mendingwall_background_application_parent_class)->finalize(self);
}

void mendingwall_background_application_class_init(MendingwallBackgroundApplicationClass* klass) {
  G_OBJECT_CLASS(klass)->dispose = mendingwall_background_application_dispose;
  G_OBJECT_CLASS(klass)->finalize = mendingwall_background_application_finalize;
}

void mendingwall_background_application_init(MendingwallBackgroundApplication* self) {
  MendingwallBackgroundApplicationPrivate* priv = mendingwall_background_application_get_instance_private(self);
  priv->session = NULL;
  priv->client_private = NULL;
}

void mendingwall_background_application_activate(MendingwallBackgroundApplication* self) {
  MendingwallBackgroundApplicationPrivate* priv = mendingwall_background_application_get_instance_private(self);

  const gchar* application_id = g_application_get_application_id(G_APPLICATION(self));
  const gchar* desktop_autostart_id = g_getenv("DESKTOP_AUTOSTART_ID");
  g_autofree const gchar* client_id = g_strdup(desktop_autostart_id ? desktop_autostart_id : "");
  g_autofree const gchar* client_path = NULL;

  /* dbus connection */
  GDBusConnection* dbus = g_application_get_dbus_connection(G_APPLICATION(self));

  /* register with org.gnome.SessionManager */
  priv->session = g_dbus_proxy_new_sync(dbus,
      G_DBUS_PROXY_FLAGS_DO_NOT_AUTO_START |
      G_DBUS_PROXY_FLAGS_DO_NOT_LOAD_PROPERTIES |
      G_DBUS_PROXY_FLAGS_DO_NOT_CONNECT_SIGNALS,
      NULL,
      "org.gnome.SessionManager",
      "/org/gnome/SessionManager",
      "org.gnome.SessionManager",
      NULL,
      NULL);
  g_autoptr(GVariant) res = g_dbus_proxy_call_sync(priv->session,
      "RegisterClient",
      g_variant_new("(ss)", application_id, client_id),
      G_DBUS_CALL_FLAGS_NONE,
      G_MAXINT,
      NULL,
      NULL);
  g_variant_get(res, "(o)", &client_path);

  /* quit on session end */
  priv->client_private = g_dbus_proxy_new_sync(dbus,
      G_DBUS_PROXY_FLAGS_NONE,
      NULL,
      "org.gnome.SessionManager",
      client_path,
      "org.gnome.SessionManager.ClientPrivate",
      NULL,
      NULL);
  g_signal_connect_swapped(
      priv->client_private,
      "g-signal::EndSession",
      G_CALLBACK(quit),
      self);

  /* keep running as background process, as application has no main window */  
  g_application_hold(G_APPLICATION(self));
}
