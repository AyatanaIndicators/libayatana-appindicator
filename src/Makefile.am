if USE_GTK3
VER=3
lib_LTLIBRARIES = libayatana-appindicator3.la
GTKGIR = Gtk-3.0
else
VER=
lib_LTLIBRARIES = libayatana-appindicator.la
GTKGIR = Gtk-2.0
endif

CLEANFILES =
DISTCLEANFILES = Makefile.in
BUILT_SOURCES =
EXTRA_DIST = \
	ayatana-appindicator-0.1.pc.in \
	ayatana-appindicator3-0.1.pc.in \
	AyatanaAppIndicator-0.1.metadata.in \
	AyatanaAppIndicator3-0.1.metadata.in

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

pkgconfig_DATA = ayatana-appindicator$(VER)-0.1.pc
pkgconfigdir = $(libdir)/pkgconfig

glib_enum_h = app-indicator-enum-types.h
glib_enum_c = app-indicator-enum-types.c
glib_enum_headers = $(addprefix $(srcdir)/, $(libayatana_appindicator_headers))

DISTCLEANFILES += app-indicator-enum-types.c

libayatana_appindicatorincludefolder=libayatana-appindicator
libayatana_appindicatorincludedir=$(includedir)/libayatana-appindicator$(VER)-0.1/$(libayatana_appindicatorincludefolder)

libayatana_appindicator_headers = \
	app-indicator.h

libayatana_appindicatorinclude_HEADERS = \
	$(libayatana_appindicator_headers) \
	$(glib_enum_h)

libayatana_appindicator_la_SOURCES = \
	$(libayatana_appindicator_headers) \
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

libayatana_appindicator_la_LDFLAGS = \
	$(COVERAGE_LDFLAGS) \
	-version-info 1:0:0 \
	-no-undefined \
	-export-symbols-regex "^[^_d].*"

libayatana_appindicator_la_CFLAGS = \
	$(LIBRARY_CFLAGS) \
	$(COVERAGE_CFLAGS) \
	-Wall -Werror -Wno-error=deprecated-declarations \
	-DG_LOG_DOMAIN=\"libayatana-appindicator\"

libayatana_appindicator_la_LIBADD = \
	$(LIBRARY_LIBS)

# We duplicate these here because Automake won't let us use $(VER) on the left hand side.
# Since we carefully use $(VER) in the right hand side above, we can assign the same values.
# Only one version of the library is every compiled at the same time, so it is safe to reuse
# the right hand sides like this.
libayatana_appindicator3includedir = $(libayatana_appindicatorincludedir)
libayatana_appindicator3include_HEADERS = $(libayatana_appindicatorinclude_HEADERS)
libayatana_appindicator3_la_SOURCES = $(libayatana_appindicator_la_SOURCES)
libayatana_appindicator3_la_LDFLAGS = $(libayatana_appindicator_la_LDFLAGS)
libayatana_appindicator3_la_CFLAGS = $(libayatana_appindicator_la_CFLAGS)
libayatana_appindicator3_la_LIBADD = $(libayatana_appindicator_la_LIBADD)

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
	$(addprefix --c-include=$(libayatana_appindicatorincludefolder)/, $(libayatana_appindicator_headers)) \
	--symbol-prefix=app \
	--identifier-prefix=App \
	--warn-all
else
INTROSPECTION_SCANNER_ARGS = \
	--add-include-path=$(srcdir) \
	$(addprefix --c-include=$(libayatana_appindicatorincludefolder)/, $(libayatana_appindicator_headers))
endif

INTROSPECTION_COMPILER_ARGS = --includedir=$(builddir)

if HAVE_INTROSPECTION

introspection_sources = \
	$(addprefix $(srcdir)/,app-indicator.c) \
	$(addprefix $(srcdir)/,$(libayatana_appindicator_headers))

AyatanaAppIndicator$(VER)-0.1.gir: libayatana-appindicator$(VER).la

AyatanaAppIndicator_0_1_gir_INCLUDES = \
    GObject-2.0 \
    $(GTKGIR)
AyatanaAppIndicator_0_1_gir_CFLAGS = $(LIBRARY_CFLAGS) -I$(srcdir) -I$(top_builddir)/src
AyatanaAppIndicator_0_1_gir_LIBS = libayatana-appindicator$(VER).la
AyatanaAppIndicator_0_1_gir_FILES = $(introspection_sources)
# AyatanaAppIndicator_0_1_gir_NAMESPACE = AyatanaAppIndicator

AyatanaAppIndicator3_0_1_gir_INCLUDES = $(AyatanaAppIndicator_0_1_gir_INCLUDES)
AyatanaAyatanaAppIndicator3_0_1_gir_CFLAGS = $(AyatanaAyatanaAppIndicator_0_1_gir_CFLAGS)
AyatanaAppIndicator3_0_1_gir_LIBS = $(AyatanaAppIndicator_0_1_gir_LIBS)
AyatanaAppIndicator3_0_1_gir_FILES = $(AyatanaAppIndicator_0_1_gir_FILES)
# AyatanaAppIndicator3_0_1_gir_NAMESPACE = AyatanaAppIndicator

INTROSPECTION_GIRS += AyatanaAppIndicator$(VER)-0.1.gir

AyatanaAppIndicator$(VER)-0.1.metadata: AyatanaAppIndicator$(VER)-0.1.gir
	cp -f $(srcdir)/$@.in $@

BUILT_SOURCES += AyatanaAppIndicator$(VER)-0.1.metadata
CLEANFILES += AyatanaAppIndicator$(VER)-0.1.metadata

girdir = $(datadir)/gir-1.0
gir_DATA = $(INTROSPECTION_GIRS)

typelibdir = $(libdir)/girepository-1.0
typelib_DATA = $(INTROSPECTION_GIRS:.gir=.typelib)

CLEANFILES += $(gir_DATA) $(typelib_DATA)

endif
