#include <config.h>
#include <mendingwall-resources.h>

#include <adwaita.h>
#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include <glib.h>

static void spawn_menus(GSettings* settings) {
  g_autofree gchar* autostart_path = g_build_filename(g_get_user_config_dir(), "autostart", NULL);
  g_autofree gchar* file_path = g_build_filename(autostart_path, "org.indii.mendingwall.menus.desktop", NULL);
  if (g_settings_get_boolean(settings, "menus")) {
    /* install autostart */
    g_autoptr(GKeyFile) key_file = g_key_file_new();
    if (g_key_file_load_from_data_dirs(key_file, "mendingwall/org.indii.mendingwall.menus.desktop", NULL, G_KEY_FILE_KEEP_COMMENTS|G_KEY_FILE_KEEP_TRANSLATIONS, NULL)) {
      g_autoptr(GFile) dir = g_file_new_for_path(autostart_path);
      g_file_make_directory_with_parents(dir, NULL, NULL);
      g_key_file_save_to_file(key_file, file_path, NULL);
    }

    /* spawn background process */
    static const gchar* argv[] = { "mendingwall-menus", "--watch", NULL };
    g_settings_sync();  // ensure current settings visible in new process
    g_spawn_async(NULL, (gchar**)argv, NULL, G_SPAWN_SEARCH_PATH, NULL, NULL, NULL, NULL);
  } else {
    /* uninstall autostart */
    g_autoptr(GFile) file = g_file_new_for_path(file_path);
    g_file_delete(file, NULL, NULL);
  }
}

static void spawn_themes(GSettings* settings) {
  g_autofree gchar* autostart_path = g_build_filename(g_get_user_config_dir(), "autostart", NULL);
  g_autofree gchar* kde_env_path = g_build_filename(g_get_user_config_dir(), "plasma-workspace", "env", NULL);
  g_autofree gchar* themes_path = g_build_filename(autostart_path, "org.indii.mendingwall.themes.desktop", NULL);
  g_autofree gchar* restore_path = g_build_filename(autostart_path, "org.indii.mendingwall.restore.desktop", NULL);
  g_autofree gchar* kde_path = g_build_filename(kde_env_path, "mendingwall-restore.sh", NULL);

  static const char* kde_contents = "#!/bin/sh\n\nmendingwall-themes --restore\n";

  if (g_settings_get_boolean(settings, "themes")) {
    /* make autostart directories in case they do not exist */
    g_autoptr(GFile) autostart_dir = g_file_new_for_path(autostart_path);
    g_autoptr(GFile) kde_env_dir = g_file_new_for_path(kde_env_path);

    g_file_make_directory_with_parents(autostart_dir, NULL, NULL);
    g_file_make_directory_with_parents(kde_env_dir, NULL, NULL);

    /* install themes autostart */
    g_autoptr(GKeyFile) themes_autostart = g_key_file_new();
    if (g_key_file_load_from_data_dirs(themes_autostart, "mendingwall/org.indii.mendingwall.themes.desktop", NULL, G_KEY_FILE_KEEP_COMMENTS|G_KEY_FILE_KEEP_TRANSLATIONS, NULL)) {
      g_key_file_save_to_file(themes_autostart, themes_path, NULL);
    }

    /* install restore autostart (used for everything but KDE) */
    g_autoptr(GKeyFile) restore_autostart = g_key_file_new();
    if (g_key_file_load_from_data_dirs(restore_autostart, "mendingwall/org.indii.mendingwall.restore.desktop", NULL, G_KEY_FILE_KEEP_COMMENTS|G_KEY_FILE_KEEP_TRANSLATIONS, NULL)) {
      g_key_file_save_to_file(restore_autostart, restore_path, NULL);
    }

    /* install restore script (used for KDE only) */
    g_autoptr(GFile) kde_file = g_file_new_for_path(kde_path);
    g_file_replace_contents(kde_file, kde_contents, strlen(kde_contents), NULL, FALSE, G_FILE_CREATE_REPLACE_DESTINATION, NULL, NULL, NULL);
    guint32 value = 0700;
    g_file_set_attribute(kde_file, G_FILE_ATTRIBUTE_UNIX_MODE, G_FILE_ATTRIBUTE_TYPE_UINT32, &value, G_FILE_QUERY_INFO_NONE, NULL, NULL);

    /* spawn background process */
    static const gchar* argv[] = { "mendingwall-themes", "--save", "--watch" };
    g_settings_sync();  // ensure current settings visible in new process
    g_spawn_async(NULL, (gchar**)argv, NULL, G_SPAWN_SEARCH_PATH, NULL, NULL, NULL, NULL);
  } else {
    /* uninstall autostart files */
    g_autoptr(GFile) themes_file = g_file_new_for_path(themes_path);
    g_autoptr(GFile) restore_file = g_file_new_for_path(restore_path);
    g_autoptr(GFile) kde_file = g_file_new_for_path(kde_path);

    g_file_delete(themes_file, NULL, NULL);
    g_file_delete(restore_file, NULL, NULL);
    g_file_delete(kde_file, NULL, NULL);
  }
}

void about(AdwApplication* app) {
  GtkBuilder* builder = gtk_builder_new_from_resource("/org/indii/mendingwall/about.ui");
  GObject* about = gtk_builder_get_object(builder, "about");
  adw_dialog_present(ADW_DIALOG(about), GTK_WIDGET(app));
}

static void activate(AdwApplication *app) {
  GtkBuilder* builder = gtk_builder_new_from_resource("/org/indii/mendingwall/mendingwall.ui");
  GObject* window = gtk_builder_get_object(builder, "mendingwall");

  /* bind gsettings */
  GSettings* settings = g_settings_new("org.indii.mendingwall");
  g_settings_bind(settings, "themes", gtk_builder_get_object(builder, "themes"), "active", G_SETTINGS_BIND_DEFAULT);
  g_settings_bind(settings, "menus", gtk_builder_get_object(builder, "menus"), "active", G_SETTINGS_BIND_DEFAULT);

  /* connect signals */
  g_signal_connect(settings, "changed::menus", G_CALLBACK(spawn_menus), window);
  g_signal_connect(settings, "changed::themes", G_CALLBACK(spawn_themes), window);

  gtk_window_set_application(GTK_WINDOW(window), GTK_APPLICATION(app));
  gtk_window_present(GTK_WINDOW(window));
  g_clear_object(&builder);
}

int main(int argc, char* argv[]) {
  g_resources_register(mendingwall_get_resource());
  AdwApplication* app = adw_application_new("org.indii.mendingwall", G_APPLICATION_DEFAULT_FLAGS);
  g_application_set_version(G_APPLICATION(app), PACKAGE_VERSION);
  g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
  return g_application_run(G_APPLICATION(app), argc, argv);
}

