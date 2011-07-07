if USE_GTK3
VER=3
else
VER=
endif

VALAFILES = indicator-example.vala
EXTRA_DIST = $(VALAFILES)

VALAFLAGS = \
	--pkg appindicator$(VER)-0.1 \
	--vapidir=$(top_builddir)/bindings/vala \
	--save-temps

if HAVE_VALAC

BUILT_SOURCES = indicator-example.c

indicator-example.c: $(VALAFILES) Makefile.am
	$(VALAC) $(VALAFLAGS) -C $< -o $@
	$(SED) -i "s|#include\s*<\s*libappindicator/app-indicator.h\s*>||g" $@

nodist_indicator_example_SOURCES = indicator-example.c
indicator_example_CFLAGS = \
	$(LIBRARY_CFLAGS) \
	-Wall \
	-I$(top_srcdir)/src \
	-include $(top_srcdir)/src/app-indicator.h
indicator_example_LDADD = \
	$(LIBRARY_LIBS) \
	$(top_builddir)/src/libappindicator$(VER).la

check_PROGRAMS = indicator-example

CLEANFILES = \
	*.c \
	indicator-example

endif
