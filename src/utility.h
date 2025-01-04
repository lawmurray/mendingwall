#include <gtk/gtk.h>

/**
 * Find descendant of a widget by name.
 *
 * @param widget Widget.
 * @param name Name.
 *
 * @return The widget if found, otherwise `NULL`.
 */
GtkWidget* find_descendant(GtkWidget* widget, const char* name);

