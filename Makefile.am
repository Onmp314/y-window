include $(top_srcdir)/build-misc/common.mk

SUBDIRS = Y libYc++ config modules clients data doc

noinst_SCRIPTS = startY.devel

ACLOCAL_AMFLAGS = -I autoconf

EXTRA_DIST = autogen.sh autoconf/test.Versions build-misc/yclpp build-misc/gentreeinfo

doc/doxygen-Y.cfg: doc/doxygen-Y.cfg.in
	make -C doc doxygen-Y.cfg

doc/doxygen-libYc++.cfg: doc/doxygen-libYc++.cfg.in
	make -C doc doxygen-libYc++.cfg

doxygen-Y: doc/doxygen-Y.cfg
	rm -rf doc/Y
	doxygen doc/doxygen-Y.cfg

doxygen-libYc++: doc/doxygen-libYc++.cfg
	rm -rf doc/libYc++
	doxygen doc/doxygen-libYc++.cfg

doxygen: doxygen-Y doxygen-libYc++

doc: doxygen

.PHONY: doc doxygen doxygen-Y doxygen-libYc++
