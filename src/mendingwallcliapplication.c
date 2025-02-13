/*
 * Copyright (C) 2025 Lawrence Murray <lawrence@indii.org>
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include <config.h>
#include <utility.h>
#include <mendingwallcliapplication.h>

struct _MendingwallCLIApplication {
  GApplication parent_instance;

  GSettings* global;
  gboolean themes, menus, restore;
};

G_DEFINE_TYPE(MendingwallCLIApplication, mendingwall_cli_application, G_TYPE_APPLICATION)

static void on_activate(MendingwallCLIApplication* self) {
  g_settings_set_boolean(self->global, "themes", self->themes);
  g_settings_set_boolean(self->global, "menus", self->menus);
  if (self->themes || self->menus) {
    launch_daemon(G_APPLICATION(self));
    install_autostart();
  } else {
    uninstall_autostart();
  }
  if (self->restore) {
    restore_themes();
  }
}

void mendingwall_cli_application_dispose(GObject* o) {
  MendingwallCLIApplication* self = MENDINGWALL_CLI_APPLICATION(o);
  g_object_unref(self->global);
  G_OBJECT_CLASS(mendingwall_cli_application_parent_class)->dispose(o);
}

void mendingwall_cli_application_finalize(GObject* o) {
  G_OBJECT_CLASS(mendingwall_cli_application_parent_class)->finalize(o);
}

void mendingwall_cli_application_class_init(MendingwallCLIApplicationClass* klass) {
  G_OBJECT_CLASS(klass)->dispose = mendingwall_cli_application_dispose;
  G_OBJECT_CLASS(klass)->finalize = mendingwall_cli_application_finalize;
}

void mendingwall_cli_application_init(MendingwallCLIApplication* self) {
  self->global = g_settings_new("org.indii.mendingwall");
  self->themes = g_settings_get_boolean(self->global, "themes");
  self->menus = g_settings_get_boolean(self->global, "menus");
  self->restore = FALSE;
}

MendingwallCLIApplication* mendingwall_cli_application_new(void) {
  MendingwallCLIApplication* self = MENDINGWALL_CLI_APPLICATION(
      g_object_new(MENDINGWALL_TYPE_CLI_APPLICATION,
          "application-id", "org.indii.mendingwall.cli",
          "version", PACKAGE_VERSION,
          "flags", G_APPLICATION_DEFAULT_FLAGS,
          NULL));

  /* command-line options */
  GOptionEntry option_entries[] = {
    { "enable-themes", 0, 0, G_OPTION_ARG_NONE, &self->themes,
        "Toggle on mend themes feature", NULL },
    { "disable-themes", 0, G_OPTION_FLAG_REVERSE, G_OPTION_ARG_NONE,
        &self->themes, "Toggle off mend themes feature", NULL },
    { "enable-menus", 0, 0, G_OPTION_ARG_NONE, &self->menus,
        "Toggle on tidy menus feature", NULL },
    { "disable-menus", 0, G_OPTION_FLAG_REVERSE, G_OPTION_ARG_NONE,
        &self->menus, "Toggle off tidy menus feature", NULL },
    { "restore", 0, 0, G_OPTION_ARG_NONE, &self->restore,
      "Restore theme from save for current desktop environment", NULL },
      G_OPTION_ENTRY_NULL
  };
  g_application_set_option_context_summary(G_APPLICATION(self),
      "- mend themes and tidy menus");
  g_application_set_option_context_description(G_APPLICATION(self),
      "For more information see https://mendingwall.indii.org");
  g_application_add_main_option_entries(G_APPLICATION(self), option_entries);

  g_signal_connect(self, "activate", G_CALLBACK(on_activate), NULL);

  return self;
}
