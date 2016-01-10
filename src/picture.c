#include "picture.h"

typedef struct {
  cairo_surface_t* surface;
  char* originalFilePath;
  char* tempFilePath;
  int width;
  int height;
} picture_t;

static GtkImage* currentPicture = NULL; // original image

static picture_t* tempPicture = NULL; // unscaled processed image

static GtkImage* uiPicture = NULL; // scaled version

static gboolean
draw_callback (GtkWidget *widget, cairo_t *cr, gpointer data)
{
  guint width, height;

  width = gtk_widget_get_allocated_width (widget);
  height = gtk_widget_get_allocated_height (widget);
  
  double wScale = 1.0;
  double hScale = 1.0;
  
  int x = 0;
  int y = 0;
  
  if (tempPicture->width > width) {
    wScale = tempPicture->width / width;
  } else {
    x = width / 2 - tempPicture->width / 2;
  }
  
  if (tempPicture->height > height) {
    hScale = tempPicture->height / height;
  } else {
    y = height / 2 - tempPicture->height / 2;
  }
  
  double scale = (wScale > hScale) ? hScale : wScale;
  
  cairo_set_source_rgb(cr, 0.1, 0.1, 0.1);
  cairo_paint(cr);
  
  if (NULL != tempPicture) {
    cairo_set_source_surface(cr, tempPicture->surface, x, y);
    cairo_rectangle(cr, x, y, width, height);
    cairo_scale(cr, scale, scale);
    cairo_fill(cr);
    cairo_paint(cr);
    return TRUE;
  }

  cairo_fill (cr);

 return FALSE;
}

GtkDrawingArea* get_picture_area() {
  GtkDrawingArea* area = gtk_drawing_area_new();
  
  gtk_widget_set_size_request (area, 1920, 1080);
  g_signal_connect (G_OBJECT (area), "draw",
                    G_CALLBACK (draw_callback), NULL);
  
  return area;
}

static void draw_pixbuf(picture_t* pic, GdkPixbuf* pixBuf) {
  cairo_t* cr = cairo_create(pic->surface);
  gdk_cairo_set_source_pixbuf(cr, pixBuf, 0, 0);
  cairo_rectangle(cr, 0, 0, pic->width, pic->height);
  cairo_fill(cr);
  cairo_destroy(cr);
}

static void set_temp_picture(GdkPixbuf* pixbuf, int width, int height) {
  if (tempPicture) {
    cairo_surface_destroy(tempPicture->surface);
    g_object_ref_sink(currentPicture);
    g_object_unref(currentPicture);
    g_free(tempPicture);
  }
  tempPicture = (picture_t*)g_malloc(sizeof(picture_t));
  tempPicture->width = gdk_pixbuf_get_width(pixbuf);
  tempPicture->height = gdk_pixbuf_get_height(pixbuf);
  tempPicture->surface = cairo_image_surface_create(CAIRO_FORMAT_RGB24, width, height);
  draw_pixbuf(tempPicture, pixbuf);
  currentPicture = gtk_image_new_from_pixbuf(pixbuf);
  g_object_unref(pixbuf);
}

void load_current_picture(char* filename) {
  GdkPixbuf * pixBuf = gdk_pixbuf_new_from_file (filename, NULL);
  set_temp_picture(g_object_ref(pixBuf), gdk_pixbuf_get_width(pixBuf), gdk_pixbuf_get_height(pixBuf));
  g_object_unref(pixBuf);
}

void set_current_picture(cairo_surface_t* surface, int width, int height) {
  if (NULL == surface) {
    load_current_picture("assets/error.png");
    return;
  }
  GdkPixbuf* pixbuf = gdk_pixbuf_get_from_surface(surface, 0, 0, width, height);
  set_temp_picture(g_object_ref(pixbuf), width, height);
  g_object_unref(pixbuf);
}

GtkImage* get_current_picture() {
  return currentPicture;
}