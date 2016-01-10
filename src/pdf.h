#include <gtk/gtk.h>
#include <poppler.h>

cairo_surface_t* get_pdf_cairo_surface(char* filename, int page, int width, int height, gpointer* document);
cairo_surface_t* get_pdf_thumbnail_cairo_surface(char* filename, int width, int height);
GtkWidget* get_pdf_toolbar(gpointer* p_doc, void(*callback)(gpointer, int, int));