#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

void add_file_item(GtkContainer* container, char* imagePath, char* tooltip, void(*callback)(void), gpointer user_data);
void add_file_item_from_surface(GtkContainer* container, cairo_surface_t* surface, char* tooltip, void(*callback)(void), gpointer user_data);