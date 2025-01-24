#pragma once

#include <adwaita.h>
#include <glib/gi18n.h>

G_BEGIN_DECLS

#define MENDINGWALL_TYPE_APPLICATION mendingwall_application_get_type()
G_DECLARE_FINAL_TYPE(MendingwallApplication, mendingwall_application, MENDINGWALL, APPLICATION, AdwApplication)

MendingwallApplication* mendingwall_application_new(void);

G_END_DECLS
