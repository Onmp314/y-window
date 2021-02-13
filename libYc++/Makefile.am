include $(top_srcdir)/build-misc/common.mk

INCLUDES += $(LIBSIGC_CPPFLAGS) -I$(top_srcdir)/libYc++

lib_LTLIBRARIES = libYc++.la

nobase_include_HEADERS = \
Y/c++.h \
Y/c++/exception.h \
Y/c++/connection.h \
Y/c++/class.h \
Y/c++/object.h \
Y/c++/message.h \
Y/c++/reply.h \
Y/c++/timer.h \
Y/c++/ticker.h \
Y/c++/widget.h \
Y/c++/window.h \
Y/c++/canvas.h \
Y/c++/label.h \
Y/c++/button.h \
Y/c++/checkbox.h \
Y/c++/console.h \
Y/c++/gridlayout.h \
Y/c++/menu.h 

noinst_HEADERS = \
Y/c++/thread_support.h

libYc___la_LIBADD  = $(LIBSIGC_LIBS)
libYc___la_SOURCES = \
Y/c++/connection.cc \
Y/c++/connection_io.cc \
Y/c++/connection_objects.cc \
Y/c++/connection_unix.cc \
Y/c++/connection_tcp.cc \
Y/c++/connection_threads.cc \
Y/c++/connection_nothreads.cc \
Y/c++/thread_support.cc \
Y/c++/class.cc \
Y/c++/object.cc \
Y/c++/message.cc \
Y/c++/reply.cc \
Y/c++/timer.cc \
Y/c++/ticker.cc \
Y/c++/window.cc \
Y/c++/canvas.cc \
Y/c++/label.cc \
Y/c++/button.cc \
Y/c++/checkbox.cc \
Y/c++/console.cc \
Y/c++/gridlayout.cc \
Y/c++/menu.cc
libYc___la_LDFLAGS = -Wl,--no-undefined
libYc___la_DEPENDENCIES = Versions
EXTRA_libYc___la_SOURCES = Versions

if CXX_VERSIONED_SYMBOLS
libYc___la_LDFLAGS += -Wl,--version-script=$(srcdir)/Versions -version-info 0:0:0
else
## Cute abuse of -release to break binary compatibility when versioned
## symbols are not available.
libYc___la_LDFLAGS += -release unversioned
endif
