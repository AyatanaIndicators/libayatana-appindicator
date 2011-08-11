if USE_GTK3
VER=3
lib_LTLIBRARIES = libappindicator3.la
GTKGIR = Gtk-3.0
else
VER=
lib_LTLIBRARIES = libappindicator.la
GTKGIR = Gtk-2.0
endif

CLEANFILES =
DISTCLEANFILES =
BUILT_SOURCES =
EXTRA_DIST = \
	appindicator-0.1.pc.in \
	appindicator3-0.1.pc.in \
	AppIndicator-0.1.metadata.in \
	AppIndicator3-0.1.metadata.in

include $(top_srcdir)/Makefile.am.enum
include $(top_srcdir)/Makefile.am.marshal

##################################
# Marshallers 
##################################

BUILT_SOURCES += \
	application-service-marshal.h \
	application-service-marshal.c

glib_marshal_list = application-service-marshal.list
glib_marshal_prefix = _application_service_marshal

##################################
# Library
##################################

pkgconfig_DATA = appindicator$(VER)-0.1.pc
pkgconfigdir = $(libdir)/pkgconfig

glib_enum_h = app-indicator-enum-types.h
glib_enum_c = app-indicator-enum-types.c
glib_enum_headers = $(addprefix $(srcdir)/, $(libappindicator_headers))

DISTCLEANFILES += app-indicator-enum-types.c

libappindicatorincludefolder=libappindicator
libappindicatorincludedir=$(includedir)/libappindicator$(VER)-0.1/$(libappindicatorincludefolder)

libappindicator_headers = \
	app-indicator.h

libappindicatorinclude_HEADERS = \
	$(libappindicator_headers) \
	$(glib_enum_h)

libappindicator_la_SOURCES = \
	$(libappindicator_headers) \
	app-indicator-enum-types.c \
	app-indicator.c \
	application-service-marshal.c \
	dbus-shared.h \
	generate-id.h \
	generate-id.c \
	gen-notification-item.xml.h \
	gen-notification-item.xml.c \
	gen-notification-watcher.xml.h \
	gen-notification-watcher.xml.c

libappindicator_la_LDFLAGS = \
	-version-info 1:0:0 \
	-no-undefined \
	-export-symbols-regex "^[^_d].*"

libappindicator_la_CFLAGS = \
	$(LIBRARY_CFLAGS) \
	-Wall -Werror \
	-DG_LOG_DOMAIN=\"libappindicator\"

libappindicator_la_LIBADD = \
	$(LIBRARY_LIBS)

# We duplicate these here because Automake won't let us use $(VER) on the left hand side.
# Since we carefully use $(VER) in the right hand side above, we can assign the same values.
# Only one version of the library is every compiled at the same time, so it is safe to reuse
# the right hand sides like this.
libappindicator3includedir = $(libappindicatorincludedir)
libappindicator3include_HEADERS = $(libappindicatorinclude_HEADERS)
libappindicator3_la_SOURCES = $(libappindicator_la_SOURCES)
libappindicator3_la_LDFLAGS = $(libappindicator_la_LDFLAGS)
libappindicator3_la_CFLAGS = $(libappindicator_la_CFLAGS)
libappindicator3_la_LIBADD = $(libappindicator_la_LIBADD)

##################################
# DBus Specs
##################################

DBUS_SPECS = \
	notification-item.xml \
	notification-watcher.xml

gen-%.xml.h: %.xml
	@echo "Building $@ from $<"
	@echo "extern const char * _$(subst -,_,$(subst .,_,$(basename $(notdir $<))));" > $@

gen-%.xml.c: %.xml
	@echo "Building $@ from $<"
	@echo "const char * _$(subst -,_,$(subst .,_,$(basename $(notdir $<)))) = " > $@
	@sed -e "s:\":\\\\\":g" -e s:^:\": -e s:\$$:\\\\n\": $< >> $@
	@echo ";" >> $@

BUILT_SOURCES += \
	gen-notification-item.xml.c \
	gen-notification-item.xml.h \
	gen-notification-watcher.xml.c \
	gen-notification-watcher.xml.h

CLEANFILES += \
	gen-notification-item.xml.c \
	gen-notification-item.xml.h \
	gen-notification-watcher.xml.c \
	gen-notification-watcher.xml.h

EXTRA_DIST += $(DBUS_SPECS)

#########################
# GObject Introsepction 
#########################

-include $(INTROSPECTION_MAKEFILE)
INTROSPECTION_GIRS = 

if INTROSPECTION_TEN
INTROSPECTION_SCANNER_ARGS = \
	--add-include-path=$(srcdir) \
	$(addprefix --c-include=$(libappindicatorincludefolder)/, $(libappindicator_headers)) \
	--symbol-prefix=app \
	--identifier-prefix=App \
	--warn-all
else
INTROSPECTION_SCANNER_ARGS = \
	--add-include-path=$(srcdir) \
	$(addprefix --c-include=$(libappindicatorincludefolder)/, $(libappindicator_headers))
endif

INTROSPECTION_COMPILER_ARGS = --includedir=$(builddir)

if HAVE_INTROSPECTION

introspection_sources = \
	$(addprefix $(srcdir)/,app-indicator.c) \
	$(addprefix $(srcdir)/,$(libappindicator_headers))

AppIndicator$(VER)-0.1.gir: libappindicator$(VER).la

AppIndicator_0_1_gir_INCLUDES = \
    GObject-2.0 \
    $(GTKGIR)
AppIndicator_0_1_gir_CFLAGS = $(LIBRARY_CFLAGS) -I$(srcdir) -I$(top_builddir)/src
AppIndicator_0_1_gir_LIBS = libappindicator$(VER).la
AppIndicator_0_1_gir_FILES = $(introspection_sources)
# AppIndicator_0_1_gir_NAMESPACE = AppIndicator

AppIndicator3_0_1_gir_INCLUDES = $(AppIndicator_0_1_gir_INCLUDES)
AppIndicator3_0_1_gir_CFLAGS = $(AppIndicator_0_1_gir_CFLAGS)
AppIndicator3_0_1_gir_LIBS = $(AppIndicator_0_1_gir_LIBS)
AppIndicator3_0_1_gir_FILES = $(AppIndicator_0_1_gir_FILES)
# AppIndicator3_0_1_gir_NAMESPACE = AppIndicator

INTROSPECTION_GIRS += AppIndicator$(VER)-0.1.gir

AppIndicator$(VER)-0.1.metadata: AppIndicator$(VER)-0.1.gir
	cp -f $(srcdir)/$@.in $@

BUILT_SOURCES += AppIndicator$(VER)-0.1.metadata
CLEANFILES += AppIndicator$(VER)-0.1.metadata

girdir = $(datadir)/gir-1.0
gir_DATA = $(INTROSPECTION_GIRS)

typelibdir = $(libdir)/girepository-1.0
typelib_DATA = $(INTROSPECTION_GIRS:.gir=.typelib)

CLEANFILES += $(gir_DATA) $(typelib_DATA)

endif
