#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

GtkDrawingArea* get_picture_area();
static GtkImage* get_current_picture();
void load_current_picture(char* filename);