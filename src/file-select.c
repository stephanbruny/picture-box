#include "file-select.h"

#define FILE_ITEM_STATE_NONE 0
#define FILE_ITEM_STATE_HOVER 1
#define FILE_ITEM_STATE_CLICK 2
#define FILE_ITEM_STATE_SELECTED 3

const int icon_width = 128;
const int icon_height = 128;

const int icon_image_width = 122;
const int icon_image_height = 122;

typedef struct {
  GtkImage* image;
  int state;
  int width;
  int height;
} file_item_t;

static gboolean
draw_callback (GtkWidget *widget, cairo_t *cr, gpointer data)
{
  file_item_t* item = (file_item_t*)data;
  g_assert(item); g_assert(item->image);
  GdkPixbuf* pixBuf = gtk_image_get_pixbuf(item->image);
  gdk_cairo_set_source_pixbuf(cr, pixBuf, icon_width/2 - icon_image_width/2, icon_height/2 - icon_image_height/2);
  cairo_rectangle(cr, 0, 0, item->width, item->height);
  if (item->state == FILE_ITEM_STATE_NONE) {
     cairo_paint_with_alpha(cr, 0.7);
     return FALSE;
  }
  
  cairo_paint(cr);
  
  return FALSE;
}

static void hover_image( GtkWidget *widget, GdkEvent* ev, file_item_t* item) {
  GdkCursor* cursor = gdk_cursor_new(GDK_HAND1);
  item->state = FILE_ITEM_STATE_HOVER;
  gdk_window_set_cursor (gtk_widget_get_parent_window(widget), cursor);
  gtk_widget_queue_draw(widget);
}

static void unhover_image( GtkWidget *widget, GdkEvent* ev, file_item_t* item) {
  gdk_window_set_cursor (gtk_widget_get_parent_window(widget), NULL);
  item->state = FILE_ITEM_STATE_NONE;
  gtk_widget_queue_draw(widget);
}

static void click_image( GtkWidget *widget, GdkEvent* ev, file_item_t* item) {
  gdk_window_set_cursor (gtk_widget_get_parent_window(widget), NULL);
  item->state = FILE_ITEM_STATE_CLICK;
  gtk_widget_queue_draw(widget);
}

static void icon_destroy( GtkWidget *widget, file_item_t* item) {
  g_print("Destroy\n");
  g_object_ref_sink(item->image);
  gtk_widget_destroy(item->image);
  g_free(item);
}

static GtkImage* create_item_icon(char* filepath) {
  GdkPixbuf * pixBuf = gdk_pixbuf_new_from_file (filepath, NULL);
  GdkPixbuf * imgBuf = gdk_pixbuf_scale_simple(pixBuf, icon_image_width, icon_image_height, GDK_INTERP_BILINEAR);
  GtkImage* result = gtk_image_new_from_pixbuf(imgBuf);
  g_object_unref(pixBuf);
  g_object_unref(imgBuf);
  return result;
}

static void add_file_item_image(GtkContainer* container, GtkImage* icon, char* tooltip, void(*callback)(void), gpointer user_data) {
  g_assert(container); g_assert(icon);
  
  GtkDrawingArea* area = gtk_drawing_area_new();
  gtk_widget_add_events (area, GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK);
  file_item_t* item = (file_item_t*)g_malloc(sizeof(file_item_t));
  
  item->width = icon_width;
  item->height = icon_height;
  item->image = icon;
  item->state = FILE_ITEM_STATE_NONE;
  
  gtk_widget_set_size_request(area, item->width, item->height);

  gtk_widget_set_tooltip_text(area, tooltip); 

  g_signal_connect (
    G_OBJECT(area), 
    "destroy",
    G_CALLBACK(icon_destroy), 
    item
  );

  g_signal_connect (
    G_OBJECT(area), 
    "button_release_event",
    G_CALLBACK(callback), 
    user_data
  );
  g_signal_connect (
    G_OBJECT(area), 
    "button_press_event",
    G_CALLBACK(click_image), 
    item
  );
  g_signal_connect (
    G_OBJECT(area), 
    "enter-notify-event",
    G_CALLBACK(hover_image),
    item
  );
  g_signal_connect (
    G_OBJECT(area), 
    "leave-notify-event",
    G_CALLBACK(unhover_image),
    item
  );
  g_signal_connect (
    G_OBJECT(area), 
    "draw",
    G_CALLBACK(draw_callback),
    item
  );
  gtk_container_add(container, area);
  gtk_widget_show_all(area);
}

void add_file_item(GtkContainer* container, char* imagePath, char* tooltip, void(*callback)(void), gpointer user_data) {
  GtkImage* icon = create_item_icon(imagePath);
  add_file_item_image(container, g_object_ref(icon), tooltip, callback, user_data);
  g_object_unref(icon);
};

void add_file_item_from_surface(GtkContainer* container, cairo_surface_t* surface, char* tooltip, void(*callback)(void), gpointer user_data) {
  GdkPixbuf* pixbuf = gdk_pixbuf_get_from_surface(surface, 0, 0, cairo_image_surface_get_width (surface), cairo_image_surface_get_height (surface));
  GdkPixbuf * imgBuf = gdk_pixbuf_scale_simple(pixbuf, icon_image_width, icon_image_height, GDK_INTERP_BILINEAR);
  GtkImage* icon = gtk_image_new_from_pixbuf(imgBuf);
  add_file_item_image(container, g_object_ref(icon), tooltip, callback, user_data);
  g_object_unref(pixbuf);
  g_object_unref(imgBuf);
  g_object_unref(icon);
}