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
#include <resources.h>
#include <mendingwallapplication.h>

#define G_SETTINGS_ENABLE_BACKEND 1
#include <gio/gio.h>
#include <gio/gsettingsbackend.h>

struct _MendingwallApplication {
  AdwApplication parent_instance;

  GSettings* global;
  GObject* window;
};

G_DEFINE_TYPE(MendingwallApplication, mendingwall_application, ADW_TYPE_APPLICATION)

static void on_changed(MendingwallApplication* self) {
  gboolean themes = g_settings_get_boolean(self->global, "themes");
  gboolean menus = g_settings_get_boolean(self->global, "menus");
  if (themes || menus) {
    launch_daemon(G_APPLICATION(self));
    install_autostart();
  } else {
    uninstall_autostart();
  }
}

static void on_about(MendingwallApplication* self) {
  g_autoptr(GtkBuilder) builder = gtk_builder_new_from_resource("/org/indii/mendingwall/about.ui");
  GObject* about = gtk_builder_get_object(builder, "about");
  adw_about_dialog_set_version(ADW_ABOUT_DIALOG(about), PACKAGE_VERSION);
  adw_dialog_present(ADW_DIALOG(about), GTK_WIDGET(self->window));
}

static void on_startup(MendingwallApplication* self) {
  /* setup */
  configure_environment();
  g_resources_register(mendingwall_get_resource());

  /* builder */
  g_autoptr(GtkBuilder) builder = gtk_builder_new_from_resource("/org/indii/mendingwall/main.ui");

  /* main window */
  self->window = gtk_builder_get_object(builder, "main");

  /* bind gsettings */
  g_settings_bind(self->global, "themes", gtk_builder_get_object(builder, "themes"), "active", G_SETTINGS_BIND_DEFAULT);
  g_settings_bind(self->global, "menus", gtk_builder_get_object(builder, "menus"), "active", G_SETTINGS_BIND_DEFAULT);

  /* connect signals */
  g_signal_connect_swapped(self->global, "changed", G_CALLBACK(on_changed), self);
  g_signal_connect_swapped(gtk_builder_get_object(builder, "about_button"), "clicked", G_CALLBACK(on_about), self);

  gtk_window_set_application(GTK_WINDOW(self->window), GTK_APPLICATION(self));
}

static void on_activate(MendingwallApplication* self) {
  on_changed(self);
  gtk_window_present(GTK_WINDOW(self->window));
}

static void mendingwall_application_dispose(GObject* o) {
  MendingwallApplication* self = MENDINGWALL_APPLICATION(o);
  if (self->global) {
    g_object_unref(self->global);
  }
  G_OBJECT_CLASS(mendingwall_application_parent_class)->dispose(o);
}

static void mendingwall_application_finalize(GObject* o) {
  G_OBJECT_CLASS(mendingwall_application_parent_class)->finalize(o);
}

static void mendingwall_application_class_init(MendingwallApplicationClass* klass) {
  G_OBJECT_CLASS(klass)->dispose = mendingwall_application_dispose;
  G_OBJECT_CLASS(klass)->finalize = mendingwall_application_finalize;
}

static void mendingwall_application_init(MendingwallApplication* self) {
  self->global = g_settings_new("org.indii.mendingwall");
  self->window = NULL;
}

MendingwallApplication* mendingwall_application_new(void) {
  MendingwallApplication* self = MENDINGWALL_APPLICATION(
      g_object_new(MENDINGWALL_TYPE_APPLICATION,
          "application-id", "org.indii.mendingwall",
          "version", PACKAGE_VERSION,
          "flags", G_APPLICATION_DEFAULT_FLAGS,
          NULL));

  g_signal_connect(self, "startup", G_CALLBACK(on_startup), NULL);
  g_signal_connect(self, "activate", G_CALLBACK(on_activate), NULL);

  return self;
}

