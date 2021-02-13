include $(top_srcdir)/build-misc/common.mk
include $(top_srcdir)/clients/clients.mk

if WANT_YITERM

bin_PROGRAMS = yiterm
noinst_HEADERS = terminal.h VTScreenView.h 
yiterm_SOURCES = terminal.cc VTScreenView.cc 
yiterm_LDADD = $(Ycxx_libs) $(LIBITERM_LIBS)

endif
