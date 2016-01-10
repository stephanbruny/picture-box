#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

GtkDrawingArea* get_picture_area();
GtkImage* get_current_picture();
void load_current_picture(char* filename);
void set_current_picture(cairo_surface_t* surface, int width, int height);