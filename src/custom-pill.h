#pragma once

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define CUSTOM_TYPE_PILL custom_pill_get_type()
G_DECLARE_FINAL_TYPE(CustomPill, custom_pill, CUSTOM, PILL, GtkWidget)

CustomPill* custom_pill_new(const gchar* label);
void custom_pill_set_label(CustomPill* self, const gchar* label);

G_END_DECLS

