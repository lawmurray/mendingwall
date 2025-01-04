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
  PROP_TITLE = 1,
  PROP_STRINGS,
  N_PROPERTIES
} CustomPillBoxProperty;

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL, };

static void custom_pill_box_set_property(GObject* object, guint property_id, const GValue* value, GParamSpec* pspec) {
  CustomPillBox* self = CUSTOM_PILL_BOX(object);

  switch ((CustomPillBoxProperty)property_id) {
  case PROP_TITLE:
    const gchar* string = g_value_get_string(value);
    GtkWidget* add_entry = find_descendant(GTK_WIDGET(self), "add_entry");
    adw_preferences_row_set_title(ADW_PREFERENCES_ROW(add_entry), string);
    break;
  case PROP_STRINGS:
    GVariant* variant = g_value_get_variant(value);
    const gchar** strings = g_variant_get_strv(variant, NULL);

    //GtkWidget* box = gtk_widget_get_first_child(GTK_WIDGET(self));
    //GtkWidget* scrolled_window = gtk_widget_get_first_child(box);
    //GtkWidget* list_view = gtk_widget_get_first_child(scrolled_window);
    //GtkSelectionModel* model = gtk_list_view_get_model(GTK_LIST_VIEW(list_view));
    //GtkStringList* string_model = GTK_STRING_LIST(gtk_no_selection_get_model(GTK_NO_SELECTION(model)));
    //guint n = g_list_model_get_n_items(G_LIST_MODEL(string_model));
    //gtk_string_list_splice(string_model, 0, n, strings);
    break;
  default:
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
  GtkStringList* model;
};

G_DEFINE_TYPE(CustomPillBox, custom_pill_box, GTK_TYPE_WIDGET)

static void restore_pill(gpointer user_data) {
  gtk_widget_remove_css_class(GTK_WIDGET(user_data), "success");
}

static void flash_pill(CustomPillBox* self, const guint pos) {
  GtkWidget* flow_box = find_descendant(GTK_WIDGET(self), "flow_box");
  GtkFlowBoxChild* flow_box_child = gtk_flow_box_get_child_at_index(GTK_FLOW_BOX(flow_box), pos);
  GtkWidget* button = gtk_flow_box_child_get_child(flow_box_child);
  gtk_widget_add_css_class(GTK_WIDGET(button), "success");
  g_timeout_add_once(150, restore_pill, button);
}

static void add_pill(GtkWidget* add_entry, gpointer user_data) {
  CustomPillBox* self = CUSTOM_PILL_BOX(user_data);
  const char* str = gtk_editable_get_text(GTK_EDITABLE(add_entry));
  if (g_strcmp0(str, "") != 0) {
    /* remove whitespace from start and end */
    char* stripped = g_strdup(str);
    g_strstrip(stripped);

    /* check if the string already exists */
    guint n = g_list_model_get_n_items(G_LIST_MODEL(self->model));
    bool already_exists = false;
    guint pos = 0;
    while (pos < n && !already_exists) {
      if (g_strcmp0(stripped, gtk_string_list_get_string(self->model, pos)) == 0) {
        /* already exists, don't add it again */
        already_exists = true;
        break;
      }
      ++pos;
    }
    if (!already_exists) {
      gtk_string_list_append(self->model, stripped);
    }

    flash_pill(self, pos);
    gtk_editable_set_text(GTK_EDITABLE(add_entry), "");
  }
}

static void remove_pill(GtkWidget* pill, gpointer user_data) {
  CustomPillBox* self = CUSTOM_PILL_BOX(user_data);
  const char* str = custom_pill_get_label(CUSTOM_PILL(pill));

  guint n = g_list_model_get_n_items(G_LIST_MODEL(self->model));
  for (guint i = 0; i < n; ++i) {
    if (g_strcmp0(str, gtk_string_list_get_string(self->model, i)) == 0) {
      gtk_string_list_remove(self->model, i);
      break;
    }
  }
}

static GtkWidget* pill_factory(void* item, gpointer user_data) {
  const gchar* label = gtk_string_object_get_string(GTK_STRING_OBJECT(item));
  CustomPillBox* self = CUSTOM_PILL_BOX(user_data);

  CustomPill* pill = custom_pill_new(label);
  g_signal_connect(pill, "clicked", G_CALLBACK(remove_pill), self);
  return GTK_WIDGET(pill);
}

static void custom_pill_box_class_init(CustomPillBoxClass* klass) {
  GObjectClass* gobject_class = G_OBJECT_CLASS(klass);

  gtk_widget_class_set_layout_manager_type(GTK_WIDGET_CLASS(klass), GTK_TYPE_BOX_LAYOUT);
  gtk_widget_class_set_template_from_resource(GTK_WIDGET_CLASS(klass), "/org/indii/mendingwall/custom-pill-box.ui");

  obj_properties[PROP_TITLE] = g_param_spec_string("title", NULL, NULL, "Add", G_PARAM_READWRITE);
  obj_properties[PROP_STRINGS] = g_param_spec_variant("strings", NULL, NULL, g_variant_type_new("as"), NULL, G_PARAM_READWRITE);
  gobject_class->set_property = custom_pill_box_set_property;
  gobject_class->get_property = custom_pill_box_get_property;
  g_object_class_install_properties(gobject_class, G_N_ELEMENTS(obj_properties), obj_properties);
}

static void custom_pill_box_init(CustomPillBox* self) {
  gtk_widget_init_template(GTK_WIDGET(self));

  /* named widgets */
  GtkWidget* flow_box = find_descendant(GTK_WIDGET(self), "flow_box");
  GtkWidget* add_entry = find_descendant(GTK_WIDGET(self), "add_entry");

  /* configure model */
  self->model = gtk_string_list_new(NULL);
  gtk_flow_box_bind_model(GTK_FLOW_BOX(flow_box), G_LIST_MODEL(self->model), pill_factory, self, NULL);

  /* configure signals */
  g_signal_connect(add_entry, "apply", G_CALLBACK(add_pill), self);
}

static void custom_pill_box_dispose(GObject* self) {
  gtk_widget_dispose_template(GTK_WIDGET(self), CUSTOM_TYPE_PILL_BOX);
  G_OBJECT_CLASS(custom_pill_box_parent_class)->dispose(self);
}

static void custom_pill_box_finalize(GObject* self) {
  G_OBJECT_CLASS(custom_pill_box_parent_class)->finalize(self);
}



