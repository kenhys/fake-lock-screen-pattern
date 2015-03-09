#ifndef _FAKE_LOCK_SCREEN_PATTERN_H_
#define _FAKE_LOCK_SCREEN_PATTERN_H_

#include <gtk/gtk.h>

#if !GTK_CHECK_VERSION(3, 0, 0)
typedef struct {
  gdouble red;
  gdouble green;
  gdouble blue;
  gdouble alpha;
} GdkRGBA;
#endif

void flsp_draw_circle(cairo_t *context,
                      gint x, gint y, gint radius,
                      GdkRGBA circle, GdkRGBA border);

#endif
