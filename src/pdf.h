#include <gtk/gtk.h>
#include <poppler.h>

cairo_surface_t* get_pdf_cairo_surface(char* filename, int page, int width, int height);