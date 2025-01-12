#include <config.h>
#include <mendingwall-themesapp.h>

struct _MendingwallThemesApp {

};

G_DEFINE_TYPE(MendingwallThemesApp, mendingwall_themes_app, G_TYPE_APPLICATION)

void mendingwall_themes_app_class_init(MendingwallThemesAppClass* klass) {

}

void mendingwall_themes_app_init(MendingwallThemesApp* self) {

}

void mendingwall_themes_app_dispose(GObject* self) {
  G_OBJECT_CLASS(mendingwall_themes_app_parent_class)->dispose(self);
}

void mendingwall_themes_app_finalize(GObject* self) {
  G_OBJECT_CLASS(mendingwall_themes_app_parent_class)->finalize(self);
}

MendingwallThemesApp* mendingwall_themes_app_new(const char* label) {
  MendingwallThemesApp* self = MENDINGWALL_THEMES_APP(g_object_new(MENDINGWALL_TYPE_THEMES_APP, NULL));
  return self;
}
