#include <stdlib.h>
#include <glib.h>
#include <gtk/gtk.h>

#include "fake-lock-screen-pattern.h"

static void
insert_textview_log(GtkWidget *view, gchar *message)
{
  GtkTextBuffer *buffer;
  GtkTextIter iter;

  buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));
  gtk_text_buffer_get_end_iter(buffer, &iter);
  gtk_text_buffer_insert(buffer, &iter, message, -1);

  gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(view),
                               &iter, 0.0, FALSE, 0, 0);
}



static gboolean
button_press_event(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
  gchar *msg;

  msg = g_strdup_printf("%s: button:%d\n", G_STRFUNC, event->button);
  insert_textview_log(GTK_WIDGET(data), msg);
  g_free(msg);

  return TRUE;
}

static gboolean
configure_event(GtkWidget *widget, GdkEventConfigure *event, gpointer data)
{
  gchar *msg;

  msg = g_strdup_printf("%s:\n", G_STRFUNC);
  insert_textview_log(GTK_WIDGET(data), msg);
  g_free(msg);

  return FALSE;
}

static gboolean
expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
  cairo_t *context;
  gchar *msg;
  gint i, j, x, y, distance, radius;
  GdkRGBA border_color = { 0.7, 0.7, 0.7 };
  GdkRGBA circle_color = { 1.0, 1.0, 1.0 };

  msg = g_strdup_printf("%s:\n", G_STRFUNC);
  insert_textview_log(GTK_WIDGET(data), msg);
  g_free(msg);

  context = gdk_cairo_create(widget->window);

  cairo_set_source_rgb(context, 0, 0, 0);
  gdk_cairo_rectangle(context, &event->area);
  cairo_fill(context);

  context = gdk_cairo_create(widget->window);

  gdk_cairo_rectangle(context, &event->area);

  cairo_set_source_rgb(context, 0, 0, 0);
  cairo_fill(context);

  distance = event->area.height / 4;
  x = event->area.width / 2 - distance;
  y = event->area.height / 4;
  radius = 10;

  for (j = 0; j < 3; j++) {
    x = event->area.width / 2 - distance;
    for (i = 0; i < 3; i++) {
      flsp_draw_circle(context, x, y, radius, circle_color, border_color);
      x += distance;
    }
    y += distance;
  }

  cairo_destroy(context);

  return TRUE;
}

static gboolean
motion_notify_event(GtkWidget *widget, GdkEventMotion *event, gpointer data)
{
  gint x, y;
  GdkModifierType modifier_state;
  gchar *msg;

  gdk_window_get_pointer(event->window, &x, &y, &modifier_state);

  msg = g_strdup_printf("%s:modifier state: %d x:%d y:%d\n",
                        G_STRFUNC, modifier_state, x, y);
  
  insert_textview_log(GTK_WIDGET(data), msg);
  g_free(msg);

  if (modifier_state & GDK_BUTTON1_MASK)
    g_print("%s:GDK_BUTTON1_MASK\n", G_STRFUNC);

  return TRUE;
}

static gchar *module = NULL;
static gchar *background = NULL;
static gchar *foreground = NULL;
static gchar *mark = NULL;
static gchar *real = NULL;
static gchar *dummy = NULL;
static gboolean verbose = FALSE;

static GOptionEntry option_entries[] = 
{
  { "pattern-module",   'p', 0, G_OPTION_ARG_STRING, &module,
    "Pattern module",  "MODULE" },
  { "real-password",    'r', 0, G_OPTION_ARG_STRING, &real,
    "Real password",   "PASSWORD" },
  { "dummy-password",   'd', 0, G_OPTION_ARG_STRING, &dummy,
    "Dummy password",  "PASSWORD" },
  { "background-color", 'b', 0, G_OPTION_ARG_STRING, &background,
    "Background color (Color name or #RRGGBB)", "COLOR" },
  { "foreground-color", 'f', 0, G_OPTION_ARG_STRING, &foreground,
    "Foreground color (Color name or #RRGGBB)", "COLOR" },
  { "mark-color",       'm', 0, G_OPTION_ARG_STRING, &mark,
    "Marked color (Color name or #RRGGBB)", "COLOR" },
  { "verbose",          'v', 0,   G_OPTION_ARG_NONE, &verbose,
    "Verbose mode", NULL },
  { NULL }
};

typedef struct _FakeLockOption {
  void *module;
  gchar *real;
  gchar *dummy;
} FakeLockOption;

static FakeLockOption option;

void load_config_file(void)
{
  GKeyFile *key_file;
  GKeyFileFlags flags;
  GError *error;
  gchar *path;

  key_file = g_key_file_new();
  path = g_build_filename(g_get_home_dir(),
                          ".fake-lock-screen-patternrc",
                          NULL);
  flags = G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS;
  g_key_file_load_from_file(key_file, path, flags, &error);
  g_key_file_free(key_file);
}

int main(int argc, char *argv[])
{
  GtkWidget *window;
  GtkWidget *drawing;
  GtkWidget *paned;
  GtkWidget *scrolled;
  GtkWidget *textview;
  GError *error = NULL;
  GOptionContext *context;

  context = g_option_context_new("- Demonstrate faked lock screen pattern program");
  g_option_context_add_main_entries(context, option_entries, "fake-lock-screen-pattern");
  if (!g_option_context_parse(context, &argc, &argv, &error)) {
    g_error("Failed to parse options: %s\n", error->message);
    exit(1);
  }

  gtk_init(&argc, &argv);

  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(window), "Fake lock screen pattern");

  gtk_widget_set_size_request(window, 400, 400);
  gtk_window_fullscreen(GTK_WINDOW(window));
  gtk_window_set_decorated(GTK_WINDOW(window), FALSE);

  drawing = gtk_drawing_area_new();

  scrolled = gtk_scrolled_window_new(NULL, NULL);
  gtk_widget_show(scrolled);

  textview = gtk_text_view_new();
  gtk_container_add(GTK_CONTAINER(scrolled), textview);

  paned = gtk_vpaned_new();
  gtk_paned_pack1(GTK_PANED(paned), drawing, TRUE, FALSE);
  gtk_paned_pack2(GTK_PANED(paned), scrolled, FALSE, FALSE);
  gtk_widget_set_size_request(scrolled, -1, 200);

  g_signal_connect(drawing, "button-press-event",
                   G_CALLBACK(button_press_event), textview);
  g_signal_connect(drawing, "configure-event",
                   G_CALLBACK(configure_event), textview);
  g_signal_connect(drawing, "expose-event",
                   G_CALLBACK(expose_event), textview);
  g_signal_connect(drawing, "motion-notify-event",
                   G_CALLBACK(motion_notify_event), textview);

  g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

  gtk_widget_set_events(drawing, gtk_widget_get_events(drawing)
                        | GDK_LEAVE_NOTIFY_MASK
                        | GDK_BUTTON_PRESS_MASK
                        | GDK_POINTER_MOTION_MASK
                        | GDK_POINTER_MOTION_HINT_MASK);

  gtk_container_add(GTK_CONTAINER(window), paned);

  gtk_widget_show_all(window);

  gtk_main();
  return 0;
}
