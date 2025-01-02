#include <config.h>
#include <mendingwall-resources.h>
#include <adwaita.h>
#include <glib/gi18n.h>

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

void scroll_to_end(gpointer user_data) {
  GtkWidget* scroller = GTK_WIDGET(user_data);
  GtkAdjustment* adjustment = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(scroller));
  double upper = gtk_adjustment_get_upper(adjustment);
  gtk_adjustment_set_value(adjustment, upper);
}

void add_exclude(GtkWidget* list_view) {
  GtkSelectionModel* model = gtk_list_view_get_model(GTK_LIST_VIEW(list_view));
  GtkStringList* string_model = GTK_STRING_LIST(gtk_no_selection_get_model(GTK_NO_SELECTION(model)));
  gtk_string_list_append(string_model, "");

  /* scroll to bottom; the size of the contents needs to be updated before this
   * works, and a short timeout suffices; g_idle_add_once() seems not to work */
  GtkWidget* scroller = gtk_widget_get_ancestor(list_view, GTK_TYPE_SCROLLED_WINDOW);
  g_timeout_add_once(20, scroll_to_end, scroller);
}

void remove_exclude(GtkWidget* label) {
  GtkWidget* list_view = gtk_widget_get_ancestor(label, GTK_TYPE_LIST_VIEW);
  GtkSelectionModel* model = gtk_list_view_get_model(GTK_LIST_VIEW(list_view));
  GtkStringList* string_model = GTK_STRING_LIST(gtk_no_selection_get_model(GTK_NO_SELECTION(model)));
  guint pos = atoi(gtk_label_get_label(GTK_LABEL(label)));
  gtk_string_list_remove(string_model, pos);
}

void activate(GtkApplication *app) {
  GtkBuilder* builder = gtk_builder_new_from_resource("/org/indii/mendingwall/main.ui");
  GObject* window = gtk_builder_get_object(builder, "main");

  /* populate exclusions */
  GObject* excludes = gtk_builder_get_object(builder, "excludes");
  const char* values[] = {"Krita", NULL};
  GtkStringList* string_model = gtk_string_list_new(values);
  GtkNoSelection* model = gtk_no_selection_new(G_LIST_MODEL(string_model));
  gtk_list_view_set_model(GTK_LIST_VIEW(excludes), GTK_SELECTION_MODEL(model));

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

