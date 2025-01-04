#include <config.h>
#include <utility.h>

GtkWidget* find_descendant(GtkWidget* widget, const char* name) {
  GtkWidget* w = widget;
  while (true) {
    if (g_strcmp0(gtk_widget_get_name(w), name) == 0) {
      return w;
    }

    GtkWidget* child = gtk_widget_get_first_child(w);
    if (child) {
      w = child;
      continue;
    }

    GtkWidget* next_sibling = gtk_widget_get_next_sibling(w);
    while (!next_sibling && w != widget) {
      w = gtk_widget_get_parent(w);
      next_sibling = gtk_widget_get_next_sibling(w);
    }
    if (next_sibling) {
      w = next_sibling;
      continue;
    } else {
      return NULL;
    }
  }
}

