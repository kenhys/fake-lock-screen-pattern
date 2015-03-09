#include <math.h>
#include <glib.h>
#include <gtk/gtk.h>
#include "fake-lock-screen-pattern.h"

void flsp_draw_circle(cairo_t *context, gint x, gint y,
                      gint radius, GdkRGBA circle, GdkRGBA border)
{
  cairo_set_line_width(context, 9);
  cairo_set_source_rgb(context, border.red, border.green, border.blue);
  cairo_arc(context, x, y, radius, 0, 2 * M_PI);
  cairo_stroke_preserve(context);
  cairo_set_source_rgb(context, circle.red, circle.green, circle.blue);
  cairo_fill(context);
}
