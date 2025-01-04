#include <config.h>
#include <custom-pill-box.h>
#include <mendingwall-resources.h>

#include <adwaita.h>
#include <gtk/gtk.h>
#include <glib/gi18n.h>

static void restore_theme(AdwAlertDialog* self, gchar* response) {
  if (strcmp(response, "restore") == 0) {
    printf("restoring default theme\n");
  }
}

void restore_theme_confirm(GtkWidget* main) {
  AdwDialog* dialog = adw_alert_dialog_new(_("Restore Default GNOME Theme?"), NULL);
  adw_alert_dialog_add_responses(ADW_ALERT_DIALOG(dialog), "cancel",  _("Cancel"), "restore", _("Restore"), NULL);
  adw_alert_dialog_set_response_appearance(ADW_ALERT_DIALOG(dialog), "restore", ADW_RESPONSE_SUGGESTED);
  adw_alert_dialog_set_default_response(ADW_ALERT_DIALOG(dialog), "cancel");
  adw_alert_dialog_set_close_response(ADW_ALERT_DIALOG(dialog), "cancel");
  g_signal_connect(dialog, "response", G_CALLBACK(restore_theme), NULL);
  adw_dialog_present(dialog, main);
}

gboolean get_mapping(GValue* value, GVariant* variant, gpointer user_data) {
  g_value_set_variant(value, variant);
  return true;
}

GVariant* set_mapping(const GValue* value, const GVariantType* expected_type, gpointer user_data) {
  return g_value_dup_variant(value);
}

void activate(GtkApplication *app) {
  GtkBuilder* builder = gtk_builder_new_from_resource("/org/indii/mendingwall/main.ui");
  GObject* window = gtk_builder_get_object(builder, "main");

  /* bind gsettings */
  GSettings* settings = g_settings_new("org.indii.mendingwall");
  g_settings_bind(settings, "preserve-themes", gtk_builder_get_object(builder, "preserve-themes"), "active", G_SETTINGS_BIND_DEFAULT);
  g_settings_bind(settings, "manage-counterparts", gtk_builder_get_object(builder, "manage-counterparts"), "enable-expansion", G_SETTINGS_BIND_DEFAULT);
  g_settings_bind_with_mapping(settings, "exclude-applications", gtk_builder_get_object(builder, "exclude-applications"), "strings", G_SETTINGS_BIND_DEFAULT, get_mapping, set_mapping, NULL, NULL);

  gtk_window_set_application(GTK_WINDOW(window), app);
  gtk_window_present(GTK_WINDOW(window));
  g_clear_object(&builder);
}

int main(int argc, char* argv[]) {
  g_resources_register(mendingwall_get_resource());
  AdwApplication* app = adw_application_new("org.indii.mendingwall", G_APPLICATION_DEFAULT_FLAGS);
  g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
  return g_application_run(G_APPLICATION(app), argc, argv);
}

