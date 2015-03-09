#include <stdlib.h>
#include <glib.h>
#include <gtk/gtk.h>

#include "fake-lock-screen-pattern.h"

gint fake[10] = { '1', '2', '5', '4', '7', '8', '9', '6', '3', '\0' };
gint input[9] = { '\0' };
gint current_index = -1;
FakeLockPatternPoint points[9];

void init_module(gint width, gint height)
{
  int i, j, index;
  int x, y, distance, radius;
  gint patterns[9] = {'1', '2', '3', '4', '5', '6', '7', '8', '9'};

  distance = height / 4;
  x = width / 2 - distance;
  y = height / 4;
  radius = 10;

  for (j = 0; j < 3; j++) {
    x = width / 2 - distance;
    for (i = 0; i < 3; i++) {
      index = j * 3 + i;
      points[index].mark = -1;
      points[index].value = patterns[index];
      points[index].top_left.x = x - radius;
      points[index].top_left.y = y - radius;
      points[index].bottom_right.x = x + radius;
      points[index].bottom_right.y = y + radius;
      x += distance;
    }
    y += distance;
  }
}

gboolean in_mark_point(gint x, gint y, gint *mark)
{
  int i;
  FakeLockPatternPoint point;

  *mark = -1;
  for (i = 0; i < 9; i++) {
    point = points[i];
    if (point.top_left.x <= x &&
        point.top_left.y <= y &&
        x <= point.bottom_right.x &&
        y <= point.bottom_right.y) {
      *mark = point.mark;
      return TRUE;
    }
  }
  return FALSE;
}

void mark_point(gint x, gint y, gint mark)
{
  int i;
  FakeLockPatternPoint point;

  for (i = 0; i < 9; i++) {
    point = points[i];
    if (point.top_left.x <= x &&
        point.top_left.y <= y &&
        x <= point.bottom_right.x &&
        y <= point.bottom_right.y) {
      points[i].mark = mark;
    }
  }
}

gboolean get_mark_point(gint x, gint y, FakeLockPatternPoint *mark)
{
  int i;

  for (i = 0; i < 9; i++) {
    if (points[i].top_left.x <= x &&
        points[i].top_left.y <= y &&
        x <= points[i].bottom_right.x &&
        y <= points[i].bottom_right.y) {
      *mark = points[i];
      return TRUE;
    }
  }
  return FALSE;
}

gboolean get_mark_point_by_index(gint index, FakeLockPatternPoint *point)
{
  int i;
  for (i = 0; i < 9; i++) {
    if (points[i].mark == index) {
      *point = points[i];
      return TRUE;
    }
  }
  return FALSE;
}

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
button_release_event(GtkWidget *widget, GdkEventButton *event, gpointer data)
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

  g_print("width: %d height: %d\n",
          widget->allocation.width,
          widget->allocation.height);
  init_module(widget->allocation.width, widget->allocation.height);

  return TRUE;
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
  gint x, y, distance, mark;
  GdkModifierType modifier_state;
  gchar *msg;
  gboolean in_bounds, succeeded;
  FakeLockPatternPoint point, prev_point;
  cairo_t *context;
  GdkRGBA border_color = { 0.7, 0.7, 0 };
  GdkRGBA circle_color = { 1.0, 0, 0 };
    
  gdk_window_get_pointer(event->window, &x, &y, &modifier_state);

#if 0
  msg = g_strdup_printf("%s:modifier state: %d x:%d y:%d\n",
                        G_STRFUNC, modifier_state, x, y);
  
  insert_textview_log(GTK_WIDGET(data), msg);
  g_free(msg);
#endif

  if (modifier_state & GDK_BUTTON1_MASK) {
    in_bounds = in_mark_point(x, y, &mark);
    if (in_bounds && mark < 0) {
      g_print("%s: MARK GDK_BUTTON1_MASK\n", G_STRFUNC);
      msg = g_strdup_printf("%s:MARK GDK_BUTTON1_MASK %d x:%d y:%d\n",
                            G_STRFUNC, modifier_state, x, y);
      insert_textview_log(GTK_WIDGET(data), msg);
      g_free(msg);

      mark_point(x, y, ++current_index);
      get_mark_point(x, y, &point);
      context = gdk_cairo_create(widget->window);
      distance = point.bottom_right.x - point.top_left.x;
      flsp_draw_circle(context,
                       point.top_left.x + distance / 2,
                       point.top_left.y + distance / 2,
                       distance / 2,
                       circle_color, border_color);
      if (current_index > 0) {
        g_print("%s: GET PREV GDK_BUTTON1_MASK\n", G_STRFUNC);
        succeeded = get_mark_point_by_index(current_index - 1, &prev_point);
        if (succeeded) {
          g_print("%s: LINE GDK_BUTTON1_MASK\n", G_STRFUNC);
          cairo_move_to(context,
                        prev_point.top_left.x + distance / 2,
                        prev_point.top_left.y + distance / 2);
          cairo_set_line_width(context, 20);
          cairo_set_source_rgb(context, circle_color.red, circle_color.green, circle_color.blue);
          cairo_line_to(context,
                        point.top_left.x + distance / 2,
                        point.top_left.y + distance / 2);
          cairo_stroke(context);
        }
      }
      cairo_destroy(context);
    }
  }

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
  g_signal_connect(drawing, "button-release-event",
                   G_CALLBACK(button_release_event), textview);
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
                        | GDK_BUTTON_RELEASE_MASK
                        | GDK_POINTER_MOTION_MASK
                        | GDK_POINTER_MOTION_HINT_MASK);

  gtk_container_add(GTK_CONTAINER(window), paned);

  gtk_widget_show_all(window);

  gtk_main();
  return 0;
}
