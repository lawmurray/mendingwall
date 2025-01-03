#include <custom-pill-box.h>
#include <custom-pill.h>
#include <utility.h>

//void add_pill(GtkWidget* container) {
//  GtkSelectionModel* model = gtk_list_view_get_model(GTK_LIST_VIEW(list_view));
//  GtkStringList* string_model = GTK_STRING_LIST(gtk_no_selection_get_model(GTK_NO_SELECTION(model)));
//  gtk_string_list_append(string_model, "");

  /* scroll to bottom; the size of the contents needs to be updated before this
   * works, and a short timeout suffices; g_idle_add_once() seems not to work */
//  GtkWidget* scroller = gtk_widget_get_ancestor(list_view, GTK_TYPE_SCROLLED_WINDOW);
//  g_timeout_add_once(20, scroll_to_end, scroller);

//  GtkWidget* self = gtk_widget_get_ancestor(list_view, CUSTOM_TYPE_PILL_BOX);
//  g_object_notify(G_OBJECT(self), "strings");
//}

typedef enum {
  PROP_STRINGS = 1,
  N_PROPERTIES
} CustomPillBoxProperty;

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL, };

static void custom_pill_box_set_property(GObject* object, guint property_id, const GValue* value, GParamSpec* pspec) {
  CustomPillBox* self = CUSTOM_PILL_BOX(object);
  if ((CustomPillBoxProperty)property_id == PROP_STRINGS) {
    GVariant* variant = g_value_get_variant(value);
    const gchar** strings = g_variant_get_strv(variant, NULL);

    //GtkWidget* box = gtk_widget_get_first_child(GTK_WIDGET(self));
    //GtkWidget* scrolled_window = gtk_widget_get_first_child(box);
    //GtkWidget* list_view = gtk_widget_get_first_child(scrolled_window);
    //GtkSelectionModel* model = gtk_list_view_get_model(GTK_LIST_VIEW(list_view));
    //GtkStringList* string_model = GTK_STRING_LIST(gtk_no_selection_get_model(GTK_NO_SELECTION(model)));
    //guint n = g_list_model_get_n_items(G_LIST_MODEL(string_model));
    //gtk_string_list_splice(string_model, 0, n, strings);
  } else {
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void custom_pill_box_get_property(GObject* object, guint property_id, GValue* value, GParamSpec* pspec) {
  CustomPillBox* self = CUSTOM_PILL_BOX(object);
  if ((CustomPillBoxProperty)property_id == PROP_STRINGS) {
    //GtkWidget* box = gtk_widget_get_first_child(GTK_WIDGET(self));
    //GtkWidget* scrolled_window = gtk_widget_get_first_child(box);
    //GtkWidget* list_view = gtk_widget_get_first_child(scrolled_window);
    //GtkSelectionModel* model = gtk_list_view_get_model(GTK_LIST_VIEW(list_view));
    //GtkStringList* string_model = GTK_STRING_LIST(gtk_no_selection_get_model(GTK_NO_SELECTION(model)));
    //guint n = g_list_model_get_n_items(G_LIST_MODEL(string_model));
    //const gchar* strv[n + 1];
    //for (guint i = 0; i < n; ++i) {
    //  strv[i] = gtk_string_list_get_string(string_model, i);
    //}
    //strv[n] = NULL;
    //GVariant* variant = g_variant_new_strv(strv, n);
    //g_value_set_variant(value, variant);
  } else {
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

struct _CustomPillBox {
  GtkWidget parent_instance;
};

G_DEFINE_TYPE(CustomPillBox, custom_pill_box, GTK_TYPE_WIDGET)

static void custom_pill_box_class_init(CustomPillBoxClass* klass) {
  GObjectClass* gobject_class = G_OBJECT_CLASS(klass);

  gtk_widget_class_set_layout_manager_type(GTK_WIDGET_CLASS(klass), GTK_TYPE_BOX_LAYOUT);
  gtk_widget_class_set_template_from_resource(GTK_WIDGET_CLASS(klass), "/org/indii/mendingwall/custom-pill-box.ui");

  obj_properties[PROP_STRINGS] = g_param_spec_variant("strings", NULL, NULL, g_variant_type_new("as"), NULL, G_PARAM_READWRITE);
  gobject_class->set_property = custom_pill_box_set_property;
  gobject_class->get_property = custom_pill_box_get_property;
  g_object_class_install_properties(gobject_class, G_N_ELEMENTS(obj_properties), obj_properties);
}

static void custom_pill_box_init(CustomPillBox* self) {
  gtk_widget_init_template(GTK_WIDGET(self));
}

static void custom_pill_box_dispose(GObject* self) {
  gtk_widget_dispose_template(GTK_WIDGET(self), CUSTOM_TYPE_PILL_BOX);
  G_OBJECT_CLASS(custom_pill_box_parent_class)->dispose(self);
}

static void custom_pill_box_finalize(GObject* self) {
  G_OBJECT_CLASS(custom_pill_box_parent_class)->finalize(self);
}

static GtkWidget* pill_factory(void* item, gpointer user_data) {
  const gchar* label = gtk_string_object_get_string(GTK_STRING_OBJECT(item));
  return GTK_WIDGET(custom_pill_new(label));
}

void custom_pill_box_bind_model(CustomPillBox* self, GListModel* model) {
  GtkWidget* flow_box = find_descendant(GTK_WIDGET(self), "flow_box");
  gtk_flow_box_bind_model(GTK_FLOW_BOX(flow_box), model, pill_factory, NULL, NULL);
}

