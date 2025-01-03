#include <config.h>
#include <mendingwall-resources.h>
#include <adwaita.h>
#include <glib/gi18n.h>
#include <custom-pill-box.h>

void restore_theme_response(AdwAlertDialog* self, gchar* response) {
  if (strcmp(response, "restore") == 0) {
    printf("restoring default theme\n");
  }
}

void restore_theme(GtkWidget* main) {
  AdwDialog* dialog = adw_alert_dialog_new(_("Restore Default GNOME Theme?"), NULL);
  adw_alert_dialog_add_responses(ADW_ALERT_DIALOG(dialog),
                                  "cancel",  _("Cancel"),
                                  "restore", _("Restore"),
                                  NULL);
  adw_alert_dialog_set_response_appearance(ADW_ALERT_DIALOG (dialog), "restore", ADW_RESPONSE_SUGGESTED);
  adw_alert_dialog_set_default_response(ADW_ALERT_DIALOG(dialog), "cancel");
  adw_alert_dialog_set_close_response(ADW_ALERT_DIALOG(dialog), "cancel");

  g_signal_connect(dialog, "response", G_CALLBACK(restore_theme_response), NULL);
  adw_dialog_present (dialog, main);
}

//void add_exclude(GtkWidget* list_view) {
//  GtkSelectionModel* model = gtk_list_view_get_model(GTK_LIST_VIEW(list_view));
//  GtkStringList* string_model = GTK_STRING_LIST(gtk_no_selection_get_model(GTK_NO_SELECTION(model)));
//  gtk_string_list_append(string_model, "");
//}

//void remove_exclude(GtkWidget* label) {
//  GtkWidget* list_view = gtk_widget_get_ancestor(label, GTK_TYPE_LIST_VIEW);
//  GtkSelectionModel* model = gtk_list_view_get_model(GTK_LIST_VIEW(list_view));
//  GtkStringList* string_model = GTK_STRING_LIST(gtk_no_selection_get_model(GTK_NO_SELECTION(model)));
//  guint pos = atoi(gtk_label_get_label(GTK_LABEL(label)));
//  gtk_string_list_remove(string_model, pos);
//}

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
  //GSettings* settings = g_settings_new("org.indii.mendingwall");
  //g_settings_bind(settings, "preserve-themes", gtk_builder_get_object(builder, "preserve-themes"), "active", G_SETTINGS_BIND_DEFAULT);
  //g_settings_bind(settings, "manage-counterparts", gtk_builder_get_object(builder, "manage-counterparts"), "enable-expansion", G_SETTINGS_BIND_DEFAULT);
  //g_settings_bind_with_mapping(settings, "excludes", model, "model", G_SETTINGS_BIND_GET, get_mapping, set_mapping, model, NULL);

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

