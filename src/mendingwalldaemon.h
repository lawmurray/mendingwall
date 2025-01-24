#pragma once

#include <gio/gio.h>

G_BEGIN_DECLS

#define MENDINGWALL_TYPE_DAEMON mendingwall_daemon_get_type()
G_DECLARE_DERIVABLE_TYPE(MendingwallDaemon, mendingwall_daemon, MENDINGWALL, DAEMON, GApplication)

struct _MendingwallDaemonClass {
  GApplicationClass parent_class;
  gpointer padding[12];
};

void mendingwall_daemon_activate(MendingwallDaemon* self);

G_END_DECLS
