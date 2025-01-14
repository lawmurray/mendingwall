#pragma once

#include <mendingwall-background-application.h>

G_BEGIN_DECLS

#define MENDINGWALL_TYPE_MENUS_APPLICATION mendingwall_menus_application_get_type()
G_DECLARE_FINAL_TYPE(MendingwallMenusApplication, mendingwall_menus_application, MENDINGWALL, MENUS_APPLICATION, MendingwallBackgroundApplication)

MendingwallMenusApplication* mendingwall_menus_application_new(void);

G_END_DECLS
