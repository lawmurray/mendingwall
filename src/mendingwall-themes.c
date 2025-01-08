#include <config.h>

#include <gio/gio.h>
#include <glib.h>
#include <glib-object.h>

void settings_changed(GSettings* settings, gchar* key, GMainLoop* loop) {
  if (g_strcmp0(key, "themes") == 0 && !g_settings_get_boolean(settings, key)) {
    g_main_loop_quit(loop);
  }
}

void gsettings_changed(GSettings* gsettings, gchar* key) {
  GValue path = G_VALUE_INIT;
  g_value_init(&path, G_TYPE_STRING);
  g_object_get_property(G_OBJECT(gsettings), "path", &path);
  g_printerr("changed: %s%s\n", g_value_get_string(&path), key);
  g_value_unset(&path);
}

void activate(GApplication *app, GMainLoop* loop) {
  g_main_loop_run(loop);
}

int main(int argc, char* argv[]) {
  /* settings */
  g_autoptr(GSettings) settings = g_settings_new("org.indii.mendingwall");

  /* check if feature is enabled */
  if (!g_settings_get_boolean(settings, "themes")) {
    return 0;
  }

  /* determine desktop environment */
  const gchar* desktop = g_getenv("XDG_CURRENT_DESKTOP");
  if (!desktop) {
    g_error("environment variable XDG_CURRENT_DESKTOP is not set");
  }

  /* process config */
  g_autoptr(GKeyFile) config = g_key_file_new();
  if (!g_key_file_load_from_data_dirs(config, "mendingwall/themes.conf", NULL, G_KEY_FILE_NONE, NULL)) {
    g_error("mendingwall/themes.conf file not found");
  }
  if (!g_key_file_has_group(config, desktop)) {
    g_error("desktop environment %s not found in mendingwall/themes.conf", desktop);
  }

  /* watch gsettings */
  gsize len = 0;
  gchar** paths = g_key_file_get_string_list(config, desktop, "GSettings", &len, NULL);
  GPtrArray* gsettings = g_ptr_array_new();
  for (guint i = 0; i < len; ++i) {
    g_printerr("watching: %s\n", paths[i]);
    GSettings* g = g_settings_new(paths[i]);
    g_signal_connect(g, "changed", G_CALLBACK(gsettings_changed), NULL);
    g_ptr_array_add(gsettings, g);
  }
  g_ptr_array_free(gsettings, TRUE);
  g_strfreev(paths);

  /* proceed */
  g_autoptr(GMainLoop) loop = g_main_loop_new(NULL, FALSE);
  g_signal_connect(settings, "changed", G_CALLBACK(settings_changed), loop);
  GApplication* app = g_application_new("org.indii.mendingwall-themes", G_APPLICATION_DEFAULT_FLAGS);
  g_signal_connect(app, "activate", G_CALLBACK(activate), loop);
  return g_application_run(G_APPLICATION(app), argc, argv);
}
