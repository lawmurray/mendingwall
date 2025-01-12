#include <config.h>
#include <mendingwall-themesapplication.h>

struct _MendingwallThemesApplication {

};

G_DEFINE_TYPE(MendingwallThemesApplication, mendingwall_themes_application, G_TYPE_APPLICATION)

void mendingwall_themes_application_class_init(MendingwallThemesApplicationClass* klass) {

}

void mendingwall_themes_application_init(MendingwallThemesApplication* self) {

}

void mendingwall_themes_application_dispose(GObject* self) {
  G_OBJECT_CLASS(mendingwall_themes_application_parent_class)->dispose(self);
}

void mendingwall_themes_application_finalize(GObject* self) {
  G_OBJECT_CLASS(mendingwall_themes_application_parent_class)->finalize(self);
}

MendingwallThemesApplication* mendingwall_themes_application_new(void) {
  MendingwallThemesApplication* self = MENDINGWALL_THEMES_APPLICATION(g_object_new(MENDINGWALL_TYPE_THEMES_APPLICATION, NULL));
  return self;
}
