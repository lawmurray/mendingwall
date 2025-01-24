#pragma once

#include <gio/gio.h>

G_BEGIN_DECLS

#define MENDINGWALL_TYPE_BACKGROUND_APPLICATION mendingwall_background_application_get_type()
G_DECLARE_DERIVABLE_TYPE(MendingwallBackgroundApplication, mendingwall_background_application, MENDINGWALL, BACKGROUND_APPLICATION, GApplication)

struct _MendingwallBackgroundApplicationClass {
  GApplicationClass parent_class;
  gpointer padding[12];
};

G_END_DECLS
