include $(top_srcdir)/build-misc/common.mk
include $(top_srcdir)/modules/module.mk

themelibdir = ${pkglibdir}/theme
themelib_LTLIBRARIES = basic.la

noinst_HEADERS = window.h draw.h font.h

basic_la_SOURCES = basic.c window.c draw.c font.c
basic_la_LDFLAGS = -module
