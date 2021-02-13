include $(top_srcdir)/build-misc/common.mk
include $(top_srcdir)/modules/module.mk

videolibdir = ${pkglibdir}/driver/video
videolib_LTLIBRARIES = sdl.la

sdl_la_SOURCES = sdl.c
sdl_la_LDFLAGS = -module 
sdl_la_LIBADD  = $(LIBSDL_LIBS)

INCLUDES += $(LIBSDL_CPPFLAGS)
