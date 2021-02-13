## This is included in every Makefile.am

INCLUDES = -I$(top_srcdir) -I$(top_builddir)
CLEANFILES = *.bb *.bbg *.gcov *.da
AM_CPPFLAGS = -D_REENTRANT
