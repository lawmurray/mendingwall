#include <custom-pill.h>
#include <utility.h>

//void remove_pill(GtkWidget* pill) {
//  GtkWidget* list_view = gtk_widget_get_ancestor(label, GTK_TYPE_LIST_VIEW);
//  GtkSelectionModel* model = gtk_list_view_get_model(GTK_LIST_VIEW(list_view));
//  GtkStringList* string_model = GTK_STRING_LIST(gtk_no_selection_get_model(GTK_NO_SELECTION(model)));
//  guint pos = atoi(gtk_label_get_label(GTK_LABEL(label)));
//  gtk_string_list_remove(string_model, pos);
//
//  GtkWidget* self = gtk_widget_get_ancestor(list_view, CUSTOM_TYPE_STRING_ARRAY_WIDGET);
//  g_object_notify(G_OBJECT(self), "strings");
//}

struct _CustomPill {
  GtkWidget parent_instance;
};

G_DEFINE_TYPE(CustomPill, custom_pill, GTK_TYPE_WIDGET)

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

CustomPill* custom_pill_new(const gchar* label) {
  CustomPill* self = CUSTOM_PILL(g_object_new(CUSTOM_TYPE_PILL, NULL));
  custom_pill_set_label(self, label);
  return self;
}

void custom_pill_set_label(CustomPill* self, const gchar* str) {
  GtkWidget* label = find_descendant(GTK_WIDGET(self), "label");
  gtk_label_set_label(GTK_LABEL(label), str);
}

