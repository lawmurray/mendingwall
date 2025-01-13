#pragma once

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define MENDINGWALL_TYPE_THEMES_APPLICATION mendingwall_themes_application_get_type()
G_DECLARE_FINAL_TYPE(MendingwallThemesApplication, mendingwall_themes_application, MENDINGWALL, THEMES_APPLICATION, GApplication)

MendingwallThemesApplication* mendingwall_themes_application_new(void);

G_END_DECLS
