#include <gtk/gtk.h>

void mendingwall_changed(GSettings* mendingwall, gchar* key, gpointer user_data) {
  GSettings* gnome = G_SETTINGS(user_data);
  printf("changed: %s\n", key);

  //g_main_loop_quit();
  //g_clear_object(&mendingwall);
  //g_clear_object(&gnome);
}

void gnome_changed(GSettings* gnome, gchar* key, gpointer user_data) {
  GSettings* mendingwall = G_SETTINGS(user_data);

    /* keys of interest in org.gnome.desktop.interface */
  static const char* keys[] = {
    "color-scheme",
    "icon-theme",
    "cursor-theme",
    "gtk-theme"
  };

  for (int i = 0; i < sizeof(keys); ++i) {
    if (strcmp(key, keys[i]) == 0) {
      printf("changed: %s\n", key);
    }
  }
}

void activate(GtkApplication *app) {
  GSettings* mendingwall = g_settings_new("org.indii.mendingwall");
  GSettings* gnome = g_settings_new("org.gnome.desktop.interface");

  g_signal_connect(mendingwall, "changed", G_CALLBACK(mendingwall_changed), gnome);
  g_signal_connect(gnome, "changed", G_CALLBACK(gnome_changed), mendingwall);

  g_autoptr(GMainLoop) loop = NULL;
  loop = g_main_loop_new (NULL, FALSE);
  g_main_loop_run (loop);
}

int main(int argc, char* argv[]) {
  GtkApplication* app = gtk_application_new("org.indii.mendingwall", G_APPLICATION_DEFAULT_FLAGS);
  g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
  return g_application_run(G_APPLICATION(app), argc, argv);
}

