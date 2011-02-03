
TESTS = indicator-example
VALAFILES = indicator-example.vala
CLEANFILES = $(TESTS) *.c
EXTRA_DIST = $(VALAFILES)

VALAFLAGS = --pkg appindicator-0.1 --vapidir=$(top_builddir)/bindings/vala --save-temps

if HAVE_VALAC
$(TESTS): $(VALAFILES) Makefile.am
	$(VALAC) $(VALAFLAGS) $(srcdir)/$(VALAFILES)

all: $(TESTS)
endif
