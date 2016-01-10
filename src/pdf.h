#include <gtk/gtk.h>
#include <poppler.h>

cairo_surface_t* get_pdf_cairo_surface(char* filename, int page, int width, int height);
cairo_surface_t* get_pdf_thumbnail_cairo_surface(char* filename, int width, int height);
GtkWidget* get_pdf_toolbar(void(*callback)(gpointer, int, int));