#include <config.h>
#include <resources.h>

#include <adwaita.h>
#include <gtk/gtk.h>
#include <gio/gio.h>
#include <glib/gi18n.h>
#include <glib.h>

static void on_changed(GSettings* settings, const gchar* key, GApplication* app) {
  g_autofree gchar* autostart_path = g_build_filename(g_get_user_config_dir(), "autostart", NULL);
  g_autofree gchar* kde_env_path = g_build_filename(g_get_user_config_dir(), "plasma-workspace", "env", NULL);
  g_autofree gchar* daemon_path = g_build_filename(autostart_path, "org.indii.mendingwalld.desktop", NULL);
  g_autofree gchar* restore_path = g_build_filename(autostart_path, "org.indii.mendingwalld.restore.desktop", NULL);
  g_autofree gchar* kde_path = g_build_filename(kde_env_path, "mendingwall-restore.sh", NULL);

  static const char* kde_contents = "#!/bin/sh\n\nmendingwalld --restore\n";

  gboolean themes_enabled = g_settings_get_boolean(settings, "themes");
  gboolean menus_enabled = g_settings_get_boolean(settings, "menus");

  if (themes_enabled || menus_enabled) {
    /* make autostart directories in case they do not exist */
    g_autoptr(GFile) autostart_dir = g_file_new_for_path(autostart_path);
    g_autoptr(GFile) kde_env_dir = g_file_new_for_path(kde_env_path);

    g_file_make_directory_with_parents(autostart_dir, NULL, NULL);
    g_file_make_directory_with_parents(kde_env_dir, NULL, NULL);

    /* install daemon autostart */
    g_autoptr(GKeyFile) daemon_autostart = g_key_file_new();
    if (g_key_file_load_from_data_dirs(daemon_autostart, "mendingwall/org.indii.mendingwalld.desktop", NULL, G_KEY_FILE_KEEP_COMMENTS|G_KEY_FILE_KEEP_TRANSLATIONS, NULL)) {
      g_key_file_save_to_file(daemon_autostart, daemon_path, NULL);
    }

    /* install restore autostart (used for everything but KDE) */
    g_autoptr(GKeyFile) restore_autostart = g_key_file_new();
    if (g_key_file_load_from_data_dirs(restore_autostart, "mendingwall/org.indii.mendingwalld.restore.desktop", NULL, G_KEY_FILE_KEEP_COMMENTS|G_KEY_FILE_KEEP_TRANSLATIONS, NULL)) {
      g_key_file_save_to_file(restore_autostart, restore_path, NULL);
    }

    /* install restore pre-start script (used for KDE only) */
    g_autoptr(GFile) kde_file = g_file_new_for_path(kde_path);
    g_file_replace_contents(kde_file, kde_contents, strlen(kde_contents), NULL, FALSE, G_FILE_CREATE_REPLACE_DESTINATION, NULL, NULL, NULL);
    guint32 value = 0700;
    g_file_set_attribute(kde_file, G_FILE_ATTRIBUTE_UNIX_MODE, G_FILE_ATTRIBUTE_TYPE_UINT32, &value, G_FILE_QUERY_INFO_NONE, NULL, NULL);

    /* spawn background process; fine if already running, new instance will
     * quit */
    g_settings_sync();  // ensure current settings visible in new process
    g_dbus_connection_call(
      g_application_get_dbus_connection(app),
      "org.indii.mendingwalld",
      "/org/indii/mendingwalld",
      "org.freedesktop.Application",
      "Activate",
      g_variant_new_parsed("({'test': <1>}, )"),
      NULL,
      G_DBUS_CALL_FLAGS_NONE,
      -1,
      NULL,
      NULL,
      NULL
    );

    /* alternatively, can launch with g_spawn_async(), but dbus preferred for
     * a consistent environment */
    //static const gchar* argv[] = { "mendingwalld", "--watch" };
    //g_spawn_async(NULL, (gchar**)argv, NULL, G_SPAWN_SEARCH_PATH, NULL, NULL, NULL, NULL);
  } else {
    /* uninstall autostart files */
    g_autoptr(GFile) daemon_file = g_file_new_for_path(daemon_path);
    g_autoptr(GFile) restore_file = g_file_new_for_path(restore_path);
    g_autoptr(GFile) kde_file = g_file_new_for_path(kde_path);

    g_file_delete(daemon_file, NULL, NULL);
    g_file_delete(restore_file, NULL, NULL);
    g_file_delete(kde_file, NULL, NULL);
  }
}

void on_about(AdwApplication* app) {
  GtkBuilder* builder = gtk_builder_new_from_resource("/org/indii/mendingwall/about.ui");
  GObject* about = gtk_builder_get_object(builder, "about");
  adw_dialog_present(ADW_DIALOG(about), GTK_WIDGET(app));
}

static void on_activate(AdwApplication* app) {
  GtkBuilder* builder = gtk_builder_new_from_resource("/org/indii/mendingwall/mendingwall.ui");
  GObject* window = gtk_builder_get_object(builder, "mendingwall");

  /* bind gsettings */
  GSettings* settings = g_settings_new("org.indii.mendingwall");
  g_settings_bind(settings, "themes", gtk_builder_get_object(builder, "themes"), "active", G_SETTINGS_BIND_DEFAULT);
  g_settings_bind(settings, "menus", gtk_builder_get_object(builder, "menus"), "active", G_SETTINGS_BIND_DEFAULT);

  /* connect signals */
  g_signal_connect(settings, "changed", G_CALLBACK(on_changed), app);

  gtk_window_set_application(GTK_WINDOW(window), GTK_APPLICATION(app));
  gtk_window_present(GTK_WINDOW(window));
  g_clear_object(&builder);
}

int main(int argc, char* argv[]) {
  g_resources_register(mendingwall_get_resource());
  AdwApplication* app = adw_application_new("org.indii.mendingwall", G_APPLICATION_DEFAULT_FLAGS);
  g_application_set_version(G_APPLICATION(app), PACKAGE_VERSION);
  g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);
  int status = g_application_run(G_APPLICATION(app), argc, argv);
  g_object_unref(app);
  return status;
}
