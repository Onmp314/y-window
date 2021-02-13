include $(top_srcdir)/build-misc/common.mk

keymapdir = $(sysconfdir)/Y/keymaps
ykbcompdir = $(pkgdatadir)/ykbcomp

CLEANFILES += ykbcomp YKB/YKBGrammar.pm gb.ykm

dist_keymap_DATA = us.Ykeymap gb.Ykeymap gb.ykb
keymap_DATA = gb.ykm
dist_ykbcomp_DATA = YKB/YKB.pm YKB/YKM.pm
ykbcomp_DATA = YKB/YKBGrammar.pm
EXTRA_DIST = YKB/YKBGrammar ykbcomp.in
bin_SCRIPTS = ykbcomp

do_subst = sed \
  -e 's,[@]pkgdatadir[@],$(pkgdatadir),g'

ykbcomp: ykbcomp.in Makefile
	rm -f $@ $@.tmp
	$(do_subst) $(srcdir)/ykbcomp.in >$@.tmp
	chmod +x $@.tmp
	chmod a-w $@.tmp
	mv -f $@.tmp $@

YKB/YKBGrammar.pm: YKB/YKBGrammar Makefile
	mkdir -p YKB
	perl -MParse::RecDescent - $< YKB::YKBGrammar
	mv -f YKBGrammar.pm YKB/YKBGrammar.pm

%.ykm: %.ykb ykbcomp $(srcdir)/YKB/YKB.pm $(srcdir)/YKB/YKM.pm Makefile ykbcomp YKB/YKBGrammar.pm
	ykbcomp_libdir=".:$(srcdir)" ./ykbcomp -o $@ $<
