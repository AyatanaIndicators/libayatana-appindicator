defsdir = $(datadir)/pygtk/2.0/defs
defs_DATA = ayatana_appindicator.defs

#CFLAGS = -Wall -Werror
INCLUDES = \
		-I$(top_srcdir)/src \
        -DG_LOG_DOMAIN=\"ayatana-appindicator-python\" \
        -DDATADIR=\"$(datadir)\" \
        -DLIBDIR=\"$(libdir)\" \
        $(APPINDICATOR_PYTHON_CFLAGS) \
        $(PYTHON_INCLUDES)

pkgpythondir = $(pyexecdir)
pkgpyexecdir = $(pyexecdir)

pkgappindicatordir = $(pkgpythondir)/ayatana_appindicator
pkgappindicator_PYTHON = __init__.py

appindicatordir = $(pkgpyexecdir)/ayatana_appindicator
appindicator_LTLIBRARIES = _ayatana_appindicator.la

_ayatana_appindicator_la_LDFLAGS = -module -avoid-version -export-symbols-regex init_ayatana_appindicator
_ayatana_appindicator_la_LIBADD = $(APPINDICATOR_PYTHON_LIBS) -L$(top_builddir)/src/.libs -layatana-appindicator
_ayatana_appindicator_la_SOURCES = ayatana_appindicatormodule.c
nodist__ayatana_appindicator_la_SOURCES = ayatana_appindicator.c

CLEANFILES = ayatana_appindicator.c
DISTCLEANFILES = Makefile.in
EXTRA_DIST = ayatana_appindicator.override.in appindicator-arg-types.py $(defs_DATA)
ayatana_appindicator.c: $(defs_DATA) ayatana_appindicator.override

%.c: %.defs
	($(PYGTK_CODEGEN) \
		--register $(PYGTK_DEFSDIR)/gtk-types.defs \
		--register $(PYGTK_DEFSDIR)/gdk-types.defs \
		--load-types $(srcdir)/appindicator-arg-types.py \
	    --override $*.override \
	    --prefix py$* $(srcdir)/$*.defs) > gen-$*.c \
	 && cp gen-$*.c $*.c \
	 && rm -f gen-$*.c
