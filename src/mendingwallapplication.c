#include <mendingwallapplication.h>
#include <foreach.h>
#include <resources.h>

struct _MendingwallApplication {
  AdwApplication parent_instance;
  
  GSettings* global;
  GObject* window;
};

G_DEFINE_TYPE(MendingwallApplication, mendingwall_application, ADW_TYPE_APPLICATION)

static void launch_daemon(MendingwallApplication* self) {
  g_settings_sync();  // ensure current settings visible in new process
  g_dbus_connection_call(
    g_application_get_dbus_connection(G_APPLICATION(self)),
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
  //static const gchar* argv[] = { "mendingwall", "--watch" };
  //g_spawn_async(NULL, (gchar**)argv, NULL, G_SPAWN_SEARCH_PATH, NULL, NULL, NULL, NULL);
}

static void on_changed(MendingwallApplication* self) {
  gboolean themes_enabled = g_settings_get_boolean(self->global, "themes");
  gboolean menus_enabled = g_settings_get_boolean(self->global, "menus");

  g_autofree gchar* autostart_path = g_build_filename(g_get_user_config_dir(), "autostart", NULL);
  g_autofree gchar* kde_env_path = g_build_filename(g_get_user_config_dir(), "plasma-workspace", "env", NULL);
  g_autofree gchar* daemon_path = g_build_filename(autostart_path, "org.indii.mendingwalld.desktop", NULL);
  g_autofree gchar* restore_path = g_build_filename(autostart_path, "org.indii.mendingwalld.restore.desktop", NULL);
  g_autofree gchar* kde_path = g_build_filename(kde_env_path, "mendingwalld.sh", NULL);

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

    /* install restore pre-start script (used for KDE only); there is no nice
     * load_from_data_dirs() type function except for keyfiles, so enumerate
     * search */
    g_autoptr(GFile) kde_from = g_file_new_build_filename(
        g_get_user_data_dir(), "mendingwall", "mendingwalld.sh", NULL);
    if (!g_file_query_exists(kde_from, NULL)) {
      foreach (dir, (const gchar**)g_get_system_data_dirs()) {
        kde_from = g_file_new_build_filename(dir, "mendingwall",
            "mendingwalld.sh", NULL);
        if (g_file_query_exists(kde_from, NULL)) {
          break;
        }
      }
    }
    if (g_file_query_exists(kde_from, NULL)) {
      g_autoptr(GFile) kde_to = g_file_new_for_path(kde_path);
      g_file_copy(kde_from, kde_to, G_FILE_COPY_OVERWRITE|G_FILE_COPY_ALL_METADATA,
          NULL, NULL, NULL, NULL);
      guint32 value = 0700;
      g_file_set_attribute(kde_to, G_FILE_ATTRIBUTE_UNIX_MODE,
          G_FILE_ATTRIBUTE_TYPE_UINT32, &value, G_FILE_QUERY_INFO_NONE, NULL,
          NULL);
    }

    /* launch daemon; fine if already running, new instance will quit */
    launch_daemon(self);
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

static void on_about(MendingwallApplication* self) {
  g_autoptr(GtkBuilder) builder = gtk_builder_new_from_resource("/org/indii/mendingwall/about.ui");
  GObject* about = gtk_builder_get_object(builder, "about");
  adw_dialog_present(ADW_DIALOG(about), GTK_WIDGET(self->window));
}

static void on_startup(MendingwallApplication* self) {
  g_autoptr(GtkBuilder) builder = gtk_builder_new_from_resource("/org/indii/mendingwall/main.ui");

  /* main window */
  self->window = gtk_builder_get_object(builder, "main");

  /* bind gsettings */
  g_settings_bind(self->global, "themes", gtk_builder_get_object(builder, "themes"), "active", G_SETTINGS_BIND_DEFAULT);
  g_settings_bind(self->global, "menus", gtk_builder_get_object(builder, "menus"), "active", G_SETTINGS_BIND_DEFAULT);

  /* connect signals */
  g_signal_connect_swapped(self->global, "changed", G_CALLBACK(on_changed), self);
  g_signal_connect_swapped(gtk_builder_get_object(builder, "about_button"), "clicked", G_CALLBACK(on_about), self);

  gtk_window_set_application(GTK_WINDOW(self->window), GTK_APPLICATION(self));
}

static void on_activate(MendingwallApplication* self) {
  launch_daemon(self);
  gtk_window_present(GTK_WINDOW(self->window));
}

void mendingwall_application_dispose(GObject* o) {
  MendingwallApplication* self = MENDINGWALL_APPLICATION(o);
  g_object_unref(self->global);
  G_OBJECT_CLASS(mendingwall_application_parent_class)->dispose(o);
}

void mendingwall_application_finalize(GObject* o) {
  G_OBJECT_CLASS(mendingwall_application_parent_class)->finalize(o);
}

void mendingwall_application_class_init(MendingwallApplicationClass* klass) {
  G_OBJECT_CLASS(klass)->dispose = mendingwall_application_dispose;
  G_OBJECT_CLASS(klass)->finalize = mendingwall_application_finalize;
}

void mendingwall_application_init(MendingwallApplication* self) {
  self->global = g_settings_new("org.indii.mendingwall");
}

MendingwallApplication* mendingwall_application_new(void) {
  MendingwallApplication* self = MENDINGWALL_APPLICATION(
      g_object_new(MENDINGWALL_TYPE_APPLICATION,
          "application-id", "org.indii.mendingwall",
          "version", PACKAGE_VERSION,
          "flags", G_APPLICATION_DEFAULT_FLAGS,
          NULL));

  g_signal_connect(self, "startup", G_CALLBACK(on_startup), NULL);
  g_signal_connect(self, "activate", G_CALLBACK(on_activate), NULL);

  return self;
}
