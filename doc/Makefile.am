## Process this file with automake to produce Makefile.in

include $(top_srcdir)/build-misc/common.mk

EXTRA_DIST =

doxygen-Y.cfg: @MAINTAINER_MODE_TRUE@ doxygen-Y.cfg.in

all-local: doxygen-Y.cfg doxygen-libYc++.cfg

clean-local:
	rm -rf Y libYc++
