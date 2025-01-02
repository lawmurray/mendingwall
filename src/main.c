#include <config.h>
#include <mendingwall-resources.h>
#include <adwaita.h>
#include <glib/gi18n.h>

void activate(GtkApplication *app) {
  GtkBuilder* builder = gtk_builder_new_from_resource("/org/indii/mendingwall/main.ui");
  GObject* window = gtk_builder_get_object(builder, "main");
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

