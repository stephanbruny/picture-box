#include "pdf.h"

typedef struct {
  int page;
  int page_count;
  PopplerDocument* doc;
  cairo_surface_t* surface;
  int width;
  int height;
  void(*on_render)(gpointer, int, int); // render request callback
  GtkWidget* page_label;
} document_page_t;

static document_page_t* currentDocument = NULL;

static void free_doc() {
  g_print("Free doc");
  g_object_unref(currentDocument->doc);
  cairo_surface_destroy(currentDocument->surface);
  g_free(currentDocument);
}

static void render_pdf_page(
  PopplerDocument* doc, 
  int n_page, 
  cairo_surface_t* surface, 
  int width, int height
) {
  PopplerPage* page = poppler_document_get_page(doc, n_page);
  cairo_t* cr = cairo_create(surface);
  cairo_rectangle(cr, 0, 0, width, height);
  cairo_set_source_rgb(cr, 1, 1, 1);
  cairo_fill(cr);
  cairo_paint(cr);
  poppler_page_render(page, cr);
  g_object_unref(page);
  cairo_destroy(cr);
}

cairo_surface_t* get_pdf_cairo_surface(char* filename, int n_page, int width, int height) {
  if (NULL != currentDocument) {
    free_doc(); 
  }
  PopplerDocument* doc;
  GError* err = NULL;
  
  gchar* uri = g_strconcat("file:", filename, NULL);
  
  doc = poppler_document_new_from_file(uri, NULL, &err);
  
  if (NULL != err) {
    g_print(err->message);
    return NULL;
  }
  
  cairo_surface_t* result = cairo_image_surface_create(CAIRO_FORMAT_RGB24, width, height);
  render_pdf_page(doc, n_page, result, width, height);

  currentDocument = (document_page_t*)g_malloc(sizeof(document_page_t));
  currentDocument->doc = doc;
  currentDocument->page = 0;
  currentDocument->page_count = poppler_document_get_n_pages(doc);
  currentDocument->surface = result;
  currentDocument->width = width;
  currentDocument->height = height;
  
  return result;
}

cairo_surface_t* get_pdf_thumbnail_cairo_surface(char* filename, int width, int height) {
  PopplerDocument* doc;
  GError* err = NULL;
  g_print("Thumbnail %s\n", filename);
  gchar* uri = g_strconcat("file:", filename, NULL);
  
  doc = poppler_document_new_from_file(uri, NULL, &err);
  
  if (NULL != err) {
    g_print(err->message);
    return NULL;
  }
  PopplerPage* page = poppler_document_get_page(doc, 0);
  cairo_surface_t* result = poppler_page_get_thumbnail(page);
  if (NULL == result) {
    result = cairo_image_surface_create(CAIRO_FORMAT_RGB24, width, height);
    render_pdf_page(doc, 0, result, width, height);
  }
  g_object_unref(doc);
  g_object_unref(page);
  return result;
}

static void get_page_label_text(GtkWidget* label, document_page_t* doc_page) {
  gchar* text = (gchar*)g_malloc(sizeof(gchar)*256);
  g_sprintf(text, "Seite %i / %i", doc_page->page + 1, doc_page->page_count);
  gtk_label_set_text(label, text);
  g_free(text);
}

static void render_doc_page(document_page_t* doc_page) {
  render_pdf_page(doc_page->doc, doc_page->page, doc_page->surface, doc_page->width, doc_page->height);
  doc_page->on_render(doc_page->surface, doc_page->width, doc_page->height);
  get_page_label_text(doc_page->page_label, doc_page);
}

static void click_prev_button( GtkWidget *widget, GdkEvent* ev, document_page_t* doc_page ) {
  if (doc_page->page > 0) {
    doc_page->page--;
  }
  render_doc_page(doc_page);
}

static void click_next_button( GtkWidget *widget, GdkEvent* ev, document_page_t* doc_page ) {
  if (doc_page->page < doc_page->page_count-1) {
    doc_page->page++;
  }
  render_doc_page(doc_page);
}

GtkWidget* get_pdf_toolbar(void(*callback)(gpointer, int, int)) {
  currentDocument->on_render = callback;
  GtkBox* vBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
  
  GtkWidget* titleLabel = gtk_label_new(poppler_document_get_title(currentDocument->doc));
  
  gtk_box_pack_start (vBox, titleLabel, FALSE, FALSE, 4);
  
  GtkBox* hBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
  
  GtkButton* button_prev = gtk_button_new_with_label("<");
  GtkWidget* label_page = gtk_label_new("PAGE");
  get_page_label_text(label_page, currentDocument);
  currentDocument->page_label = label_page;
  GtkButton* button_next = gtk_button_new_with_label(">");
  
  g_signal_connect (button_prev, "clicked",
		      G_CALLBACK (click_prev_button), currentDocument);
  
  g_signal_connect (button_next, "clicked",
		      G_CALLBACK (click_next_button), currentDocument);
  
  gtk_box_pack_start (hBox, button_prev, FALSE, FALSE, 2);
  gtk_box_pack_start (hBox, label_page, FALSE, FALSE, 2);
  gtk_box_pack_start (hBox, button_next, FALSE, FALSE, 2);
  
  gtk_box_pack_start (vBox, hBox, FALSE, FALSE, 4);
  
  return vBox;
}