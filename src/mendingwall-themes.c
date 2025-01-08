#include <config.h>

#include <gio/gio.h>
#include <glib.h>
#include <glib-object.h>

void settings_changed(GSettings* settings, gchar* key, GMainLoop* loop) {
  if (g_strcmp0(key, "themes") == 0 && !g_settings_get_boolean(settings, key)) {
    /* feature has been disabled, terminate */
    g_main_loop_quit(loop);
  }
}

void activate(GApplication *app) {
  g_autoptr(GSettings) settings = g_settings_new("org.indii.mendingwall");
  if (g_settings_get_boolean(settings, "themes")) {
    /* feature is enabled, start */
    g_autoptr(GMainLoop) loop = g_main_loop_new(NULL, FALSE);
    g_signal_connect(settings, "changed", G_CALLBACK(settings_changed), loop);
    g_main_loop_run(loop);
  }
}

int main(int argc, char* argv[]) {
  GApplication* app = g_application_new("org.indii.mendingwall-themes", G_APPLICATION_DEFAULT_FLAGS);
  g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
  return g_application_run(G_APPLICATION(app), argc, argv);
}
