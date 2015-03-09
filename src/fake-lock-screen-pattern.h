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

typedef struct _FakeLockOption {
  void *module;
  gchar *real;
  gchar *dummy;
} FakeLockOption;

typedef struct {
  gint mark;
  gint value;
  GdkPoint top_left;
  GdkPoint bottom_right;
} FakeLockPatternPoint;

typedef enum {
  FLSP_ONE_STROKE,
  FLSP_ON_BOARD,
  FLSP_N_PATTERN,
} FakeLockScreenPattern;

void flsp_draw_circle(cairo_t *context,
                      gint x, gint y, gint radius,
                      GdkRGBA circle, GdkRGBA border);

gboolean is_marked(gint x, gint y,
                   gint *marked, void *user_data);

#endif
