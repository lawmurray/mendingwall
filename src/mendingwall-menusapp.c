#include <config.h>
#include <mendingwall-menusapp.h>

struct _MendingwallMenusApp {

};

G_DEFINE_TYPE(MendingwallMenusApp, mendingwall_menus_app, G_TYPE_APPLICATION)

void mendingwall_menus_app_class_init(MendingwallMenusAppClass* klass) {

}

void mendingwall_menus_app_init(MendingwallMenusApp* self) {

}

void mendingwall_menus_app_dispose(GObject* self) {
  G_OBJECT_CLASS(mendingwall_menus_app_parent_class)->dispose(self);
}

void mendingwall_menus_app_finalize(GObject* self) {
  G_OBJECT_CLASS(mendingwall_menus_app_parent_class)->finalize(self);
}

MendingwallMenusApp* mendingwall_menus_app_new(const char* label) {
  MendingwallMenusApp* self = MENDINGWALL_MENUS_APP(g_object_new(MENDINGWALL_TYPE_MENUS_APP, NULL));
  return self;
}
