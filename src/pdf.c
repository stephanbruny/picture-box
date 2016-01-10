#include "pdf.h"

cairo_surface_t* get_pdf_cairo_surface(char* filename, int n_page, int width, int height) {
  PopplerDocument* doc;
  PopplerPage* page;
  GError* err = NULL;
  
  gchar* uri = g_strconcat("file:", filename, NULL);
  
  doc = poppler_document_new_from_file(uri, NULL, &err);
  
  if (NULL != err) {
    g_print(err->message);
    return NULL;
  }
  
  cairo_surface_t* result = cairo_image_surface_create(CAIRO_FORMAT_RGB24, width, height);
  cairo_t* cr = cairo_create(result);
  cairo_rectangle(cr, 0, 0, width, height);
  cairo_set_source_rgb(cr, 1, 1, 1);
  cairo_fill(cr);
  cairo_paint(cr);
  page = poppler_document_get_page(doc, n_page);
  poppler_page_render(page, cr);
  // cairo_paint(cr);
  g_object_unref(doc);
  g_object_unref(page);
  
  return result;
}