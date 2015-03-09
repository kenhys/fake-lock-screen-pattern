pattern3x3dir = $(pkglibdir)/3x3

pattern3x3_LTLIBRARIES = libpattern3x3.la

libpattern3x3_la_SOURCES = pattern3x3.c

libpattern3x3_la_LDFLAGS = -L../../plugins

libpattern3x3_la_CFLAGS = \
	$(GTK_CFLAGS) \
	$(GLIB_CFLAGS) \
	-I$(srcdir)/../../src

libpattern3x3_la_LIBADD = \
	$(GTK_LIBS) \
	$(GLIB_LIBS)
