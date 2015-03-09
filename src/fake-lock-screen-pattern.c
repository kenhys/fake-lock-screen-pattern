#include <stdlib.h>
#include <glib.h>
#include <gtk/gtk.h>

#include "fake-lock-screen-pattern.h"

gint input[9] = { '\0' };
gint current_index = -1;
FakeLockPatternPoint points[9];

static gchar *module = NULL;
static gchar *background = NULL;
static gchar *fake = NULL;
static gchar *real = NULL;
static gboolean debug = FALSE;
static gboolean verbose = FALSE;

static FakeLockScreenPatternOption option;

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

void draw_background_pattern(GdkWindow *window, const GdkRectangle *rectangle,
                             const GdkRGBA fill_color, const GdkRGBA border_color)
{
  cairo_t *context;
  int i, j, x, y, distance, radius;

  context = gdk_cairo_create(window);
  gdk_cairo_rectangle(context, rectangle);

  cairo_set_source_rgb(context, 0, 0, 0);
  cairo_fill(context);

  gdk_cairo_rectangle(context, rectangle);

  cairo_set_source_rgb(context, 0, 0, 0);
  cairo_fill(context);

  distance = rectangle->height / 4;
  x = rectangle->width / 2 - distance;
  y = rectangle->height / 4;
  radius = 10;

  for (j = 0; j < 3; j++) {
    x = rectangle->width / 2 - distance;
    for (i = 0; i < 3; i++) {
      flsp_draw_circle(context, x, y, radius, fill_color, border_color);
      x += distance;
    }
    y += distance;
  }

  cairo_destroy(context);

}

gchar *get_marked_string()
{
  gchar *buf;
  int i, j, max;

  for (i = 0; i < 9; i++) {
    if (points[i].mark > 0) {
      if (max < points[i].mark) {
        max = points[i].mark;
      }
    }
  }
  buf = g_new0(gchar, max + 2);
  buf[max+1] = '\0';
  for (i = 0; i < max + 1; i++) {
    for (j = 0; j < 9; j++) {
      if (points[j].mark == i) {
        buf[i] = points[j].value;
      }
    }
  }
  g_print("MARK %s\n", buf);
  return buf;
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
key_press_event(GtkWidget *widget, GdkEventKey  *event, gpointer   user_data)
{
  g_print("%s\n", G_STRFUNC);
  return TRUE;
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
  GdkRectangle rectangle;
  GdkRGBA border_color = { 0.7, 0.7, 0.7 };
  GdkRGBA circle_color = { 1.0, 1.0, 1.0 };

  msg = g_strdup_printf("%s: button:%d\n", G_STRFUNC, event->button);
  insert_textview_log(GTK_WIDGET(data), msg);
  g_free(msg);

  msg = get_marked_string();
  insert_textview_log(GTK_WIDGET(data), msg);

  g_print("mark:%s dummy:%s", msg, fake);
  if (g_strcmp0(msg, fake) == 0) {
    gtk_main_quit();
  }
  g_free(msg);

  rectangle.width = option.width;
  rectangle.height = option.height;
  draw_background_pattern(widget->window, &rectangle,
                          circle_color, border_color);
  current_index = -1;
  init_module(option.width, option.height);

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

  option.width = widget->allocation.width;
  option.height = widget->allocation.height;

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

  draw_background_pattern(widget->window, &event->area,
                          circle_color, border_color);

  return TRUE;
}

static gboolean
motion_notify_event(GtkWidget *widget, GdkEventMotion *event, gpointer data)
{
  gint x, y, distance, mark;
  GdkModifierType modifier_state;
  gchar *msg;
  gboolean in_bounds, prev_exist = FALSE;
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
    context = gdk_cairo_create(widget->window);
    if (current_index >= 0) {
      g_print("%s: GET PREV GDK_BUTTON1_MASK\n", G_STRFUNC);
      prev_exist = get_mark_point_by_index(current_index, &prev_point);
    }
    if (in_bounds && mark < 0) {
      g_print("%s: MARK GDK_BUTTON1_MASK\n", G_STRFUNC);
      msg = g_strdup_printf("%s:MARK GDK_BUTTON1_MASK %d x:%d y:%d\n",
                            G_STRFUNC, modifier_state, x, y);
      insert_textview_log(GTK_WIDGET(data), msg);
      g_free(msg);

      mark_point(x, y, ++current_index);
      get_mark_point(x, y, &point);
      distance = point.bottom_right.x - point.top_left.x;
      flsp_draw_circle(context,
                       point.top_left.x + distance / 2,
                       point.top_left.y + distance / 2,
                       distance / 2,
                       circle_color, border_color);
      if (prev_exist) {
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

        cairo_save(context);
      }
    } else {
      if (prev_exist) {
        cairo_restore(context);
        cairo_move_to(context,
                      prev_point.top_left.x + distance / 2,
                      prev_point.top_left.y + distance / 2);
        cairo_set_line_width(context, 20);
        cairo_set_source_rgb(context, circle_color.red, circle_color.green, circle_color.blue);
        cairo_line_to(context, x, y);
        cairo_stroke(context);
      }
    }
    cairo_destroy(context);
  }

  return TRUE;
}

static GOptionEntry option_entries[] = 
{
  { "pattern-module",   'm', 0, G_OPTION_ARG_STRING, &module,
    "Pattern module",   "MODULE" },
  { "real-password",    'r', 0, G_OPTION_ARG_STRING, &real,
    "Real password",    "PASSWORD" },
  { "fake-password",    'f', 0, G_OPTION_ARG_STRING, &fake,
    "Fake password",    "PASSWORD" },
  { "background-color", 'b', 0, G_OPTION_ARG_STRING, &background,
    "Background color (Color name or #RRGGBB)", "COLOR" },
  { "debug",            'd', 0, G_OPTION_ARG_NONE, &debug,
    "Debug mode", "" },
  { "verbose",          'v', 0, G_OPTION_ARG_NONE, &verbose,
    "Verbose mode", NULL },
  { NULL }
};

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
  gtk_window_set_keep_above(GTK_WINDOW(window), TRUE);

  drawing = gtk_drawing_area_new();

  scrolled = gtk_scrolled_window_new(NULL, NULL);
  gtk_widget_show(scrolled);

  textview = gtk_text_view_new();
  gtk_container_add(GTK_CONTAINER(scrolled), textview);

  paned = gtk_vpaned_new();
  gtk_paned_pack1(GTK_PANED(paned), drawing, TRUE, FALSE);
  if (debug) {
    gtk_paned_pack2(GTK_PANED(paned), scrolled, FALSE, FALSE);
    gtk_widget_set_size_request(scrolled, -1, 200);
  }

  g_signal_connect(drawing, "button-press-event",
                   G_CALLBACK(button_press_event), textview);
  g_signal_connect(drawing, "button-release-event",
                   G_CALLBACK(button_release_event), textview);
  g_signal_connect(window, "key-press-event",
                   G_CALLBACK(key_press_event), textview);
  g_signal_connect(drawing, "configure-event",
                   G_CALLBACK(configure_event), textview);
  g_signal_connect(drawing, "expose-event",
                   G_CALLBACK(expose_event), textview);
  g_signal_connect(drawing, "motion-notify-event",
                   G_CALLBACK(motion_notify_event), textview);

  g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

  gtk_widget_set_events(drawing, gtk_widget_get_events(drawing)
                        | GDK_LEAVE_NOTIFY_MASK
                        | GDK_KEY_PRESS_MASK
                        | GDK_BUTTON_PRESS_MASK
                        | GDK_BUTTON_RELEASE_MASK
                        | GDK_POINTER_MOTION_MASK
                        | GDK_POINTER_MOTION_HINT_MASK);

  gtk_container_add(GTK_CONTAINER(window), paned);

  gtk_widget_show_all(window);

  gtk_main();
  return 0;
}
