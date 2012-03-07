defsdir = $(datadir)/pygtk/2.0/defs
defs_DATA = appindicator.defs

#CFLAGS = -Wall -Werror
INCLUDES = \
		-I$(top_srcdir)/src \
        -DG_LOG_DOMAIN=\"appindicator-python\" \
        -DDATADIR=\"$(datadir)\" \
        -DLIBDIR=\"$(libdir)\" \
        $(APPINDICATOR_PYTHON_CFLAGS) \
        $(PYTHON_INCLUDES)

pkgpythondir = $(pyexecdir)
pkgpyexecdir = $(pyexecdir)

pkgappindicatordir = $(pkgpythondir)/appindicator
pkgappindicator_PYTHON = __init__.py

appindicatordir = $(pkgpyexecdir)/appindicator
appindicator_LTLIBRARIES = _appindicator.la

_appindicator_la_LDFLAGS = -module -avoid-version -export-symbols-regex init_appindicator
_appindicator_la_LIBADD = $(APPINDICATOR_PYTHON_LIBS) -L$(top_builddir)/src/.libs -lappindicator
_appindicator_la_SOURCES = appindicatormodule.c
nodist__appindicator_la_SOURCES = appindicator.c

CLEANFILES = appindicator.c
EXTRA_DIST = appindicator.override.in appindicator-arg-types.py $(defs_DATA)
appindicator.c: $(defs_DATA) appindicator.override

%.c: %.defs
	($(PYGTK_CODEGEN) \
		--register $(PYGTK_DEFSDIR)/gtk-types.defs \
		--register $(PYGTK_DEFSDIR)/gdk-types.defs \
		--load-types $(srcdir)/appindicator-arg-types.py \
	    --override $*.override \
	    --prefix py$* $(srcdir)/$*.defs) > gen-$*.c \
	 && cp gen-$*.c $*.c \
	 && rm -f gen-$*.c
