
VALAFILES = indicator-example.vala
EXTRA_DIST = $(VALAFILES)

VALAFLAGS = --pkg appindicator-0.1 --vapidir=$(top_builddir)/bindings/vala --save-temps

if HAVE_VALAC
indicator-example: $(VALAFILES) Makefile.am
	$(VALAC) $(VALAFLAGS) $(srcdir)/$(VALAFILES)

check: indicator-example

CLEANFILES = \
	*.c \
	indicator-example

endif
