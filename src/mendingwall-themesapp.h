#pragma once

#include <gio/gio.h>
#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define MENDINGWALL_TYPE_THEMES_APP mendingwall_themes_app_get_type()
G_DECLARE_FINAL_TYPE(MendingwallThemesApp, mendingwall_themes_app, MENDINGWALL, THEMES_APP, GApplication)

MendingwallThemesApp* mendingwall_themes_app_new(const char* label);

G_END_DECLS
