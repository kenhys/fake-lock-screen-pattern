#include <stdlib.h>
#include <glib.h>
#include <gtk/gtk.h>

static gboolean
button_press_event(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
  g_print("%s:%d\n", G_STRFUNC, event->button);

  return TRUE;
}

static gboolean
configure_event(GtkWidget *widget, GdkEvent *event, gpointer data)
{
  g_print("%s:%d\n", G_STRFUNC, event->button);
  return FALSE;
}

static gboolean
expose_event(GtkWidget *widget, GdkEvent *event, gpointer data)
{
  g_print("%s:%d\n", G_STRFUNC, event->button);
  return FALSE;
}

static gboolean
motion_notify_event(GtkWidget *widget, GdkEventMotion *event, gpointer data)
{
  gint x, y;
  GdkModifierType modifier_state;

  gdk_window_get_pointer(event->window, &x, &y, &modifier_state);

  g_print("%s:modifier state: %d x:%d y:%d\n", G_STRFUNC, modifier_state, x, y);

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

int main(int argc, char *argv[])
{
  GtkWidget *window;
  GtkWidget *drawing;
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

  gtk_window_fullscreen(GTK_WINDOW(window));
  gtk_window_set_decorated(GTK_WINDOW(window), FALSE);

  drawing = gtk_drawing_area_new();
  gtk_container_add(GTK_CONTAINER(window), drawing);

  g_signal_connect(drawing, "button-press-event",
                   G_CALLBACK(button_press_event), NULL);
  g_signal_connect(drawing, "configure-event",
                   G_CALLBACK(configure_event), NULL);
  g_signal_connect(drawing, "expose-event",
                   G_CALLBACK(expose_event), NULL);
  g_signal_connect(drawing, "motion-notify-event",
                   G_CALLBACK(motion_notify_event), NULL);

  gtk_widget_set_events(drawing, gtk_widget_get_events(drawing)
                        | GDK_LEAVE_NOTIFY_MASK
                        | GDK_BUTTON_PRESS_MASK
                        | GDK_POINTER_MOTION_MASK
                        | GDK_POINTER_MOTION_HINT_MASK);

  gtk_widget_show_all(window);

  g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

  gtk_main();
  return 0;
}
