#include <config.h>
#include <mendingwall-resources.h>

#include <adwaita.h>
#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include <glib.h>

static void restore_theme(AdwAlertDialog* self, gchar* response) {
  if (strcmp(response, "restore") == 0) {
    g_print("restoring default theme\n");
  }
}

void restore_theme_confirm(GtkWidget* mendingwall) {
  AdwDialog* dialog = adw_alert_dialog_new(_("Restore Default GNOME Theme?"), NULL);
  adw_alert_dialog_add_responses(ADW_ALERT_DIALOG(dialog), "cancel",  _("Cancel"), "restore", _("Restore"), NULL);
  adw_alert_dialog_set_response_appearance(ADW_ALERT_DIALOG(dialog), "restore", ADW_RESPONSE_SUGGESTED);
  adw_alert_dialog_set_default_response(ADW_ALERT_DIALOG(dialog), "cancel");
  adw_alert_dialog_set_close_response(ADW_ALERT_DIALOG(dialog), "cancel");
  g_signal_connect(dialog, "response", G_CALLBACK(restore_theme), NULL);
  adw_dialog_present(dialog, mendingwall);
}

void spawn_menus(GSettings* settings) {
  g_autofree gchar* dir_path = g_build_filename(g_get_user_config_dir(), "autostart", NULL);
  g_autofree gchar* file_path = g_build_filename(dir_path, "org.indii.mendingwall.menus.desktop", NULL);
  if (g_settings_get_boolean(settings, "menus")) {
    /* install autostart */
    g_autoptr(GKeyFile) key_file = g_key_file_new();
    if (g_key_file_load_from_data_dirs(key_file, "mendingwall/org.indii.mendingwall.menus.desktop", NULL, G_KEY_FILE_KEEP_COMMENTS|G_KEY_FILE_KEEP_TRANSLATIONS, NULL)) {
      g_autoptr(GFile) dir = g_file_new_for_path(dir_path);
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

void spawn_themes(GSettings* settings) {
  g_autofree gchar* dir_path = g_build_filename(g_get_user_config_dir(), "autostart", NULL);
  g_autofree gchar* save_file_path = g_build_filename(dir_path, "org.indii.mendingwall.themes.save.desktop", NULL);
  g_autofree gchar* restore_file_path = g_build_filename(dir_path, "org.indii.mendingwall.themes.restore.desktop", NULL);
  if (g_settings_get_boolean(settings, "themes")) {
    /* install restore autostart */
    g_autoptr(GKeyFile) restore_key_file = g_key_file_new();
    if (g_key_file_load_from_data_dirs(restore_key_file, "mendingwall/org.indii.mendingwall.themes.restore.desktop", NULL, G_KEY_FILE_KEEP_COMMENTS|G_KEY_FILE_KEEP_TRANSLATIONS, NULL)) {
      g_autoptr(GFile) dir = g_file_new_for_path(dir_path);
      g_file_make_directory_with_parents(dir, NULL, NULL);
      g_key_file_save_to_file(restore_key_file, restore_file_path, NULL);
    }

    /* install save autostart */
    g_autoptr(GKeyFile) save_key_file = g_key_file_new();
    if (g_key_file_load_from_data_dirs(save_key_file, "mendingwall/org.indii.mendingwall.themes.save.desktop", NULL, G_KEY_FILE_KEEP_COMMENTS|G_KEY_FILE_KEEP_TRANSLATIONS, NULL)) {
      g_autoptr(GFile) dir = g_file_new_for_path(dir_path);
      g_file_make_directory_with_parents(dir, NULL, NULL);
      g_key_file_save_to_file(save_key_file, save_file_path, NULL);
    }

    /* spawn background process */
    static const gchar* argv[] = { "mendingwall-themes-save", NULL };
    g_settings_sync();  // ensure current settings visible in new process
    g_spawn_async(NULL, (gchar**)argv, NULL, G_SPAWN_SEARCH_PATH, NULL, NULL, NULL, NULL);
  } else {
    /* uninstall autostart */
    g_autoptr(GFile) save_file = g_file_new_for_path(save_file_path);
    g_autoptr(GFile) restore_file = g_file_new_for_path(restore_file_path);
    g_file_delete(save_file, NULL, NULL);
    g_file_delete(restore_file, NULL, NULL);
  }
}

void activate(GtkApplication *app) {
  GtkBuilder* builder = gtk_builder_new_from_resource("/org/indii/mendingwall/mendingwall.ui");
  GObject* window = gtk_builder_get_object(builder, "mendingwall");

  /* bind gsettings */
  GSettings* settings = g_settings_new("org.indii.mendingwall");
  g_settings_bind(settings, "themes", gtk_builder_get_object(builder, "themes"), "active", G_SETTINGS_BIND_DEFAULT);
  g_settings_bind(settings, "menus", gtk_builder_get_object(builder, "menus"), "active", G_SETTINGS_BIND_DEFAULT);

  /* connect signals */
  g_signal_connect(settings, "changed::menus", G_CALLBACK(spawn_menus), window);
  g_signal_connect(settings, "changed::themes", G_CALLBACK(spawn_themes), window);

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

