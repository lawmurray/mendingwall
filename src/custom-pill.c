#include <custom-pill.h>
#include <utility.h>

struct _CustomPill {
  GtkButton parent_instance;
};

G_DEFINE_TYPE(CustomPill, custom_pill, GTK_TYPE_BUTTON)

static void custom_pill_class_init(CustomPillClass* klass) {
  gtk_widget_class_set_layout_manager_type(GTK_WIDGET_CLASS(klass), GTK_TYPE_BOX_LAYOUT);
  gtk_widget_class_set_template_from_resource(GTK_WIDGET_CLASS(klass), "/org/indii/mendingwall/custom-pill.ui");
}

static void custom_pill_init(CustomPill* self) {
  gtk_widget_init_template(GTK_WIDGET(self));
}

static void custom_pill_dispose(GObject* self) {
  gtk_widget_dispose_template(GTK_WIDGET(self), CUSTOM_TYPE_PILL);
  G_OBJECT_CLASS(custom_pill_parent_class)->dispose(self);
}

static void custom_pill_finalize(GObject* self) {
  G_OBJECT_CLASS(custom_pill_parent_class)->finalize(self);
}

CustomPill* custom_pill_new(const char* label) {
  CustomPill* self = CUSTOM_PILL(g_object_new(CUSTOM_TYPE_PILL, NULL));
  custom_pill_set_label(self, label);
  return self;
}

const char* custom_pill_get_label(CustomPill* self) {
  GtkWidget* label = find_descendant(GTK_WIDGET(self), "label");
  return gtk_label_get_label(GTK_LABEL(label));
}

void custom_pill_set_label(CustomPill* self, const char* str) {
  GtkWidget* label = find_descendant(GTK_WIDGET(self), "label");
  gtk_label_set_label(GTK_LABEL(label), str);
}

