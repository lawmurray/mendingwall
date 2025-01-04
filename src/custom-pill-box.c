#include <custom-pill-box.h>
#include <custom-pill.h>
#include <utility.h>

typedef enum {
  PROP_TITLE = 1,
  PROP_STRINGS,
  N_PROPERTIES
} CustomPillBoxProperty;

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL, };

struct _CustomPillBox {
  GtkBox parent_instance;
  GtkStringList* model;
};

G_DEFINE_TYPE(CustomPillBox, custom_pill_box, GTK_TYPE_BOX)

static void unflash_pill(gpointer user_data) {
  gtk_widget_remove_css_class(GTK_WIDGET(user_data), "success");
}

static void flash_pill(CustomPillBox* self, const guint pos) {
  GtkWidget* flow_box = find_descendant(GTK_WIDGET(self), "flow-box");
  GtkFlowBoxChild* flow_box_child = gtk_flow_box_get_child_at_index(GTK_FLOW_BOX(flow_box), pos);
  GtkWidget* button = gtk_flow_box_child_get_child(flow_box_child);
  gtk_widget_add_css_class(GTK_WIDGET(button), "success");
  g_timeout_add_once(150, unflash_pill, button);
}

static void add_pill(GtkWidget* add_entry, gpointer user_data) {
  CustomPillBox* self = CUSTOM_PILL_BOX(user_data);
  const char* str = gtk_editable_get_text(GTK_EDITABLE(add_entry));
  if (g_strcmp0(str, "") != 0) {
    /* remove whitespace from start and end */
    gchar* stripped = g_strdup(str);
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
      g_object_notify(G_OBJECT(self), "strings");
    }
    gtk_editable_set_text(GTK_EDITABLE(add_entry), "");
    flash_pill(self, pos);
    g_free(stripped);
  }
}

static void remove_pill(GtkWidget* pill, gpointer user_data) {
  CustomPillBox* self = CUSTOM_PILL_BOX(user_data);
  const char* str = custom_pill_get_label(CUSTOM_PILL(pill));
  guint n = g_list_model_get_n_items(G_LIST_MODEL(self->model));
  for (guint i = 0; i < n; ++i) {
    if (g_strcmp0(str, gtk_string_list_get_string(self->model, i)) == 0) {
      gtk_string_list_remove(self->model, i);
      g_object_notify(G_OBJECT(self), "strings");
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

static void custom_pill_box_set_property(GObject* object, guint property_id, const GValue* value, GParamSpec* pspec) {
  CustomPillBox* self = CUSTOM_PILL_BOX(object);
  if (property_id == PROP_TITLE) {
    GtkWidget* add_entry = find_descendant(GTK_WIDGET(self), "add-entry");
    const char* string = g_value_get_string(value);
    adw_preferences_row_set_title(ADW_PREFERENCES_ROW(add_entry), string);
  } else if (property_id == PROP_STRINGS) {
    GVariant* variant = g_value_get_variant(value);
    const gchar** strv = g_variant_get_strv(variant, NULL);
    guint n = g_list_model_get_n_items(G_LIST_MODEL(self->model));
    gtk_string_list_splice(self->model, 0, n, strv);
    g_free(strv);
  } else {
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void custom_pill_box_get_property(GObject* object, guint property_id, GValue* value, GParamSpec* pspec) {
  CustomPillBox* self = CUSTOM_PILL_BOX(object);
  if (property_id == PROP_TITLE) {
    GtkWidget* add_entry = find_descendant(GTK_WIDGET(self), "add-entry");
    const char* string = adw_preferences_row_get_title(ADW_PREFERENCES_ROW(add_entry));
    g_value_set_string(value, string);
  } else if (property_id == PROP_STRINGS) {
    guint n = g_list_model_get_n_items(G_LIST_MODEL(self->model));
    const char* strv[n + 1];
    for (guint i = 0; i < n; ++i) {
      strv[i] = gtk_string_list_get_string(self->model, i);
    }
    strv[n] = NULL;
    g_value_set_variant(value, g_variant_new_strv(strv, n));
  } else {
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

void custom_pill_box_class_init(CustomPillBoxClass* klass) {
  GObjectClass* gobject_class = G_OBJECT_CLASS(klass);

  gtk_widget_class_set_layout_manager_type(GTK_WIDGET_CLASS(klass), GTK_TYPE_BOX_LAYOUT);
  gtk_widget_class_set_template_from_resource(GTK_WIDGET_CLASS(klass), "/org/indii/mendingwall/custom-pill-box.ui");

  GVariantType* strv_type = g_variant_type_new("as");
  obj_properties[PROP_TITLE] = g_param_spec_string("title", NULL, NULL, "Add", G_PARAM_READWRITE);
  obj_properties[PROP_STRINGS] = g_param_spec_variant("strings", NULL, NULL, strv_type, NULL, G_PARAM_READWRITE);
  gobject_class->set_property = custom_pill_box_set_property;
  gobject_class->get_property = custom_pill_box_get_property;
  g_object_class_install_properties(gobject_class, G_N_ELEMENTS(obj_properties), obj_properties);
  g_free(strv_type);
}

void custom_pill_box_init(CustomPillBox* self) {
  gtk_widget_init_template(GTK_WIDGET(self));

  /* named widgets */
  GtkWidget* flow_box = find_descendant(GTK_WIDGET(self), "flow-box");
  GtkWidget* add_entry = find_descendant(GTK_WIDGET(self), "add-entry");

  /* configure model */
  self->model = gtk_string_list_new(NULL);
  gtk_flow_box_bind_model(GTK_FLOW_BOX(flow_box), G_LIST_MODEL(self->model), pill_factory, self, NULL);

  /* configure signals */
  g_signal_connect(add_entry, "apply", G_CALLBACK(add_pill), self);
}

void custom_pill_box_dispose(GObject* self) {
  gtk_widget_dispose_template(GTK_WIDGET(self), CUSTOM_TYPE_PILL_BOX);
  G_OBJECT_CLASS(custom_pill_box_parent_class)->dispose(self);
}

void custom_pill_box_finalize(GObject* self) {
  G_OBJECT_CLASS(custom_pill_box_parent_class)->finalize(self);
}



