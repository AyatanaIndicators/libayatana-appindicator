if USE_GTK3
VER=3
else
VER=
endif

VALAFILES = ayatana-indicator-example.vala
EXTRA_DIST = $(VALAFILES)

VALAFLAGS = \
	--pkg ayatana-appindicator$(VER)-0.1 \
	--vapidir=$(top_builddir)/bindings/vala \
	--save-temps

if HAVE_VALAC

BUILT_SOURCES = ayatana-indicator-example.c

ayatana-indicator-example.c: $(VALAFILES) Makefile.am
	$(VALAC) $(VALAFLAGS) -C $< -o $@
	$(SED) -i "s|#include\s*<\s*libayatana-appindicator/app-indicator.h\s*>||g" $@

nodist_ayatana_indicator_example_SOURCES = ayatana-indicator-example.c
ayatana_indicator_example_CFLAGS = \
	$(LIBRARY_CFLAGS) \
	-Wall \
	-I$(top_srcdir)/src \
	-include $(top_srcdir)/src/app-indicator.h
ayatana_indicator_example_LDADD = \
	$(LIBRARY_LIBS) \
	$(top_builddir)/src/libayatana-appindicator$(VER).la

check_PROGRAMS = ayatana-indicator-example

CLEANFILES = \
	*.c \
	ayatana-indicator-example
DISTCLEANFILES = Makefile.in

endif
