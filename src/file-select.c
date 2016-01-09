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

static GtkImage* create_item_icon(char* filepath) {
  GdkPixbuf * pixBuf = gdk_pixbuf_new_from_file (filepath, NULL);
  GdkPixbuf * imgBuf = gdk_pixbuf_scale_simple(pixBuf, icon_image_width, icon_image_height, GDK_INTERP_BILINEAR);
  return gtk_image_new_from_pixbuf(imgBuf);
}

void add_file_item(GtkContainer* container, char* imagePath, char* tooltip, void(*callback)(void), gpointer user_data) {
  GtkDrawingArea* area = gtk_drawing_area_new();
  gtk_widget_add_events (area, GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK);
  file_item_t* item = (file_item_t*)g_malloc(sizeof(file_item_t));
  
  GtkImage* icon = create_item_icon(imagePath);
  
  item->width = icon_width;
  item->height = icon_height;
  item->image = icon;
  item->state = FILE_ITEM_STATE_NONE;
  
  gtk_widget_set_size_request(area, item->width, item->height);

  gtk_widget_set_tooltip_text(area, tooltip); 

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
};