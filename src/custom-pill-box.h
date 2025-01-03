#pragma once

#include <adwaita.h>

G_BEGIN_DECLS

#define CUSTOM_TYPE_PILL_BOX custom_pill_box_get_type()
G_DECLARE_FINAL_TYPE(CustomPillBox, custom_pill_box, CUSTOM, PILL_BOX, GtkBox)

CustomPillBox* custom_pill_box_new(void);
void custom_pill_box_bind_model(CustomPillBox* self, GListModel* model);

G_END_DECLS

