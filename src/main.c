#include <gio/gio.h>
#include <limits.h>
#include <stdlib.h>

#include "picture.h"
#include "file-select.h"
#include "pdf.h"

/* static local ui elements */
static GtkWindow* _mainWindow = NULL;
static GtkBox* imageGrid = NULL;
static GtkLabel* infoLabel = NULL;
static GtkDrawingArea* pictureArea = NULL;
static GtkBox* jobPicturesBox = NULL;
static GtkScrolledWindow* toolbox = NULL;

char* root_mount_dir;

#define A4_WIDTH_72 595
#define A4_HEIGHT_72 842

typedef struct {
  char* path;
  char* name;
} folder_t;

typedef struct {
  folder_t*  prev;
  folder_t*  data;
} folder_node_t;

/* local definitions */
static void open_dir(folder_node_t* parent, char* path, char* name);
static void on_render_pdf(cairo_surface_t* surface, int width, int height);

static void hello( GtkWidget *widget,
                   gpointer   data )
{
    g_print ("Hello World\n");
}

static gboolean delete_event( GtkWidget *widget,
                              GdkEvent  *event,
                              gpointer   data )
{
    g_print ("delete event occurred\n");

    return FALSE;
}

static void clear_container(GtkContainer* container) {
  GList *children, *iter;

  children = gtk_container_get_children(GTK_CONTAINER(container));
  for(iter = children; iter != NULL; iter = g_list_next(iter))
    gtk_widget_destroy(GTK_WIDGET(iter->data));
  g_list_free(children);
}

/* Another callback */
static void destroy( GtkWidget *widget, gpointer data )
{
    gtk_main_quit ();
}

static void click_image( GtkWidget *widget, GdkEvent* ev, gchar* filename )
{
  load_current_picture(filename);
  gtk_widget_queue_draw (pictureArea);
}

static void click_pdf ( GtkWidget *widget, GdkEvent* ev, gchar* filename ) {
  gpointer* doc = (gpointer*)g_malloc(sizeof(gpointer));
  set_current_picture(get_pdf_cairo_surface(filename, 0, A4_WIDTH_72, A4_HEIGHT_72, doc), A4_WIDTH_72, A4_HEIGHT_72);
  
  clear_container(toolbox);
  
  gtk_container_add(GTK_CONTAINER(toolbox), get_pdf_toolbar(doc, on_render_pdf));
  
  gtk_widget_queue_draw (pictureArea);
  
  gtk_widget_show_all(toolbox);
}

static void click_folder( GtkWidget *widget, GdkEvent* ev, folder_node_t* folder )
{
  open_dir(folder->prev, folder->data->path, folder->data->name);
}

static void show_error_message(GtkWindow* parent, char* message) {
  GtkWidget * msgBox = gtk_message_dialog_new (parent,
                        GTK_DIALOG_MODAL,
                        GTK_MESSAGE_ERROR,
                        GTK_BUTTONS_OK,
                        message);
  gtk_dialog_run (GTK_DIALOG (msgBox));
  gtk_widget_destroy (msgBox);
}

const int imageGridSize = 128;

static void add_image(char* path, char* filename) {
  gchar* filePath = g_strconcat(path, "/", filename, NULL);
  add_file_item(imageGrid, filePath, filename, click_image, filePath);
}

static void add_folder(folder_node_t* node, char* icon) {
  const char* folderIcon = icon ? icon : "/usr/share/pixmaps/gnome-folder.png"; // TODO: Make this non-magic!
  g_print("Adding folder %s\n", node->data->path);
  add_file_item(imageGrid, folderIcon, node->data->name ? node->data->name : node->data->path, click_folder, node);
}

static void add_pdf(char* path, char* filename) {
  const char* pdfIcon = "assets/pdf.png";
  gchar* filePath = g_strconcat(path, "/", filename, NULL);
  cairo_surface_t* surf = get_pdf_thumbnail_cairo_surface(filePath, 128, 128);
  if (NULL == surf) {
    surf = get_pdf_cairo_surface(filePath, 0, A4_WIDTH_72, A4_HEIGHT_72, NULL);
  }
  if (NULL == surf) {
    add_file_item(imageGrid, pdfIcon, filename, click_pdf, filePath);
    return;
  }
  add_file_item_from_surface(imageGrid, surf, filename, click_pdf, filePath);
}

int startsWith(const char *pre, const char *str)
{
    size_t lenpre = strlen(pre),
           lenstr = strlen(str);
    return lenstr < lenpre ? 0 : strncmp(pre, str, lenpre) == 0;
}



folder_node_t* createFolder(folder_node_t* parent, char* path, char* name) {
  folder_node_t* newNode = (folder_node_t*)g_malloc(sizeof(folder_node_t));
  
  newNode->data = (folder_t*)g_malloc(sizeof(folder_t));
  newNode->data->path = (parent) ? g_strconcat(parent->data->path, "/", name, NULL) : path;
  newNode->data->name = name ? name : path;
  newNode->prev = parent;
  
  return newNode;
}

static void open_dir(folder_node_t* parent, char* path, char* name) {
  GDir *dir;
  GError *error = NULL;
  const gchar *filename;
  
  folder_node_t* newNode = createFolder(parent, path, name);
  
  // char* dirPath = path ? g_build_filename (root, path, NULL) : root;
  dir = g_dir_open(path, 0, &error);
  if (error) {
    g_print("ERROR\n");
    show_error_message(_mainWindow, error->message);
  }
  clear_container((GtkContainer*)imageGrid);
  
  if (parent) {
    add_folder(parent, "./assets/back.png");
  }
  
  while ((filename = g_dir_read_name(dir))) {
    // Check for directory
    const char* filePath = g_strconcat(path, "/", filename, NULL);
    if (g_file_test(filePath, G_FILE_TEST_IS_DIR)) {
      add_folder(createFolder(newNode, filePath, filename), "./assets/folder.png");
      continue;
    }
    // Check for image files
    gboolean is_certain = FALSE;
    char *content_type = g_content_type_guess (filename, NULL, 0, &is_certain);
    if (content_type != NULL)
    {
      char *mime_type = g_content_type_get_mime_type (content_type);
      g_print(mime_type);
      g_print("\n");
      if (startsWith("image/", (const char*)mime_type)) {
        add_image(path, filename);
      }
      if (startsWith("application/pdf", (const char*)mime_type)) {
        add_pdf(path, filename);
      }
      g_free(mime_type);
    }
    g_free(content_type);
  }
  gtk_widget_hide(infoLabel);
  gtk_widget_show(imageGrid);
}

static void on_mount_added (GVolumeMonitor *volume_monitor,
               GMount          *mount,
               gpointer        user_data) {
  GFile* newMountRoot = g_mount_get_root(mount);
  g_print("Added mount ");
  g_print(g_file_get_path(newMountRoot));
  g_print("\n");
  root_mount_dir = g_file_get_path(newMountRoot);
  open_dir(NULL, root_mount_dir, "Speichergerät");
}


static void init_device_control() {
  GVolumeMonitor * monitor = g_volume_monitor_get ();
  
  g_signal_connect (monitor, "mount-added", G_CALLBACK (on_mount_added), NULL);
  
}

static void on_render_pdf(cairo_surface_t* surface, int width, int height) {
  set_current_picture(surface, width, height);
  gtk_widget_queue_draw (pictureArea);
}

int main( int   argc,
          char *argv[] )
{
    GtkWidget *window;
    GtkWidget *button;

    gtk_init (&argc, &argv);
    
    /* create a new window */
    _mainWindow = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    
    gtk_window_set_default_size(_mainWindow, 1920, 1080);
   
    g_signal_connect (_mainWindow, "delete-event",
		      G_CALLBACK (delete_event), NULL);
    
    g_signal_connect (_mainWindow, "destroy",
		      G_CALLBACK (destroy), NULL);
    

    gtk_container_set_border_width (GTK_CONTAINER (_mainWindow), 10);
    

    button = gtk_button_new_with_label ("Beenden");

    g_signal_connect (button, "clicked",
		      G_CALLBACK (hello), NULL);
   
    g_signal_connect_swapped (button, "clicked",
			      G_CALLBACK (gtk_widget_destroy),
                              _mainWindow);
                              
    GtkBox* contentBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    GtkBox* pictureBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    jobPicturesBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    pictureArea = get_picture_area();
    toolbox = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_min_content_width(toolbox, 216);
    
    // gtk_box_pack_start (pictureBox, jobPicturesBox, TRUE, TRUE, 4);
    gtk_box_pack_start (pictureBox, pictureArea, TRUE, TRUE, 4);
    gtk_box_pack_start (pictureBox, toolbox, TRUE, TRUE, 4);
    
    imageGrid = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    gtk_grid_set_column_spacing(imageGrid, 4);
    
    infoLabel = gtk_label_new ("Bitte Speichermedium einführen");
    
    GtkWidget* scrollBox = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add (GTK_CONTAINER (scrollBox), imageGrid);
    
    gtk_box_pack_start (contentBox, pictureBox, TRUE, TRUE, 4);
    gtk_box_pack_start (contentBox, infoLabel, TRUE, TRUE, 4);
    gtk_box_pack_start (contentBox, scrollBox, TRUE, TRUE, 4);
    // gtk_box_pack_start (contentBox, button, TRUE, TRUE, 4);

    gtk_container_add (GTK_CONTAINER (_mainWindow), contentBox);
    
    gtk_widget_show_all (_mainWindow);
    gtk_widget_hide(imageGrid);
    gtk_widget_set_margin_left ((GtkWidget*)imageGrid, 4);
    
    load_current_picture("./assets/default.png");
    
    init_device_control();
    
    //gtk_window_fullscreen (_mainWindow);

    gtk_main ();
    
    return 0;
}