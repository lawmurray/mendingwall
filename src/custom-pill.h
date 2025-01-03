#pragma once

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define CUSTOM_TYPE_PILL custom_pill_get_type()
G_DECLARE_FINAL_TYPE(CustomPill, custom_pill, CUSTOM, PILL, GtkButton)

CustomPill* custom_pill_new(const char* label);
void custom_pill_set_label(CustomPill* self, const char* label);
const char* custom_pill_get_label(CustomPill* self);

G_END_DECLS

