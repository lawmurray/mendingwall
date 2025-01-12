#pragma once

#include <gio/gio.h>
#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define MENDINGWALL_TYPE_MENUS_APP mendingwall_menus_app_get_type()
G_DECLARE_FINAL_TYPE(MendingwallMenusApp, mendingwall_menus_app, MENDINGWALL, MENUS_APP, GApplication)

MendingwallMenusApp* mendingwall_menus_app_new(const char* label);

G_END_DECLS
