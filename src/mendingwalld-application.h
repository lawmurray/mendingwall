#pragma once

#include <mendingwall-background-application.h>

G_BEGIN_DECLS

#define MENDINGWALL_TYPE_D_APPLICATION mendingwalld_application_get_type()
G_DECLARE_FINAL_TYPE(MendingwallDApplication, mendingwalld_application, MENDINGWALL, D_APPLICATION, MendingwallBackgroundApplication)

MendingwallDApplication* mendingwalld_application_new(void);

G_END_DECLS
