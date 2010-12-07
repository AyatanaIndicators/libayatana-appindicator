if USE_GTK3
VER=3
lib_LTLIBRARIES = libappindicator3.la
GTKGIR = Gtk-3.0
GTKVAPI = gtk+-3.0
else
VER=
lib_LTLIBRARIES = libappindicator.la
GTKGIR = Gtk-2.0
GTKVAPI = gtk+-2.0
endif

CLEANFILES =
DISTCLEANFILES =
BUILT_SOURCES =
EXTRA_DIST = \
	appindicator-0.1.pc.in \
	appindicator3-0.1.pc.in \
	AppIndicator-0.1.metadata

include $(top_srcdir)/Makefile.am.enum
include $(top_srcdir)/Makefile.am.marshal

##################################
# Indicator
##################################

applicationlibdir = $(INDICATORDIR)
applicationlib_LTLIBRARIES = libapplication.la
libapplication_la_SOURCES = \
	application-service-marshal.c \
	dbus-shared.h \
	indicator-application.c
libapplication_la_CFLAGS = $(INDICATOR_CFLAGS) \
	-Wall \
	-Wl,-Bsymbolic-functions \
	-Wl,-z,defs \
	-Wl,--as-needed \
	-Werror \
	-DG_LOG_DOMAIN=\"Indicator-Application\"
libapplication_la_LIBADD = $(INDICATOR_LIBS)
libapplication_la_LDFLAGS = -module -avoid-version

##################################
# Service
##################################

libexec_PROGRAMS = indicator-application-service

BUILT_SOURCES += \
	application-service-server.h \
	application-service-marshal.h \
	application-service-marshal.c \
	dbus-properties-client.h \
	notification-item-client.h \
	notification-watcher-server.h

indicator_application_service_SOURCES = \
	application-service.c \
	application-service-appstore.h \
	application-service-appstore.c \
	application-service-marshal.c \
	application-service-watcher.h \
	application-service-watcher.c \
	app-indicator-enum-types.c \
	dbus-shared.h \
	generate-id.h \
	generate-id.c

indicator_application_service_CFLAGS = \
	$(INDICATOR_CFLAGS) \
	-DDATADIR="\"$(pkgdatadir)\"" \
	-Wall -Werror \
	-DG_LOG_DOMAIN=\"indicator-application-service\"

indicator_application_service_LDADD = \
	$(INDICATOR_LIBS) \
	libappindicator$(VER).la

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

libappindicatorincludedir=$(includedir)/libappindicator$(VER)-0.1/libappindicator

libappindicator_headers = \
	app-indicator.h

libappindicatorinclude_HEADERS = \
	$(libappindicator_headers) \
	$(glib_enum_h)

BUILT_SOURCES += \
	notification-watcher-client.h \
	notification-item-server.h

libappindicator_la_SOURCES = \
	$(libappindicator_headers) \
	app-indicator-enum-types.c \
	app-indicator.c \
	application-service-marshal.c \
	generate-id.h \
	generate-id.c

libappindicator_la_LDFLAGS = \
	-version-info 1:0:0 \
	-no-undefined \
	-export-symbols-regex "^[^_d].*"

libappindicator_la_CFLAGS = \
	$(INDICATOR_CFLAGS) \
	-Wall -Werror \
	-DG_LOG_DOMAIN=\"libappindicator\"

libappindicator_la_LIBADD = \
	$(INDICATOR_LIBS)

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
	dbus-properties.xml \
	application-service.xml \
	notification-approver.xml \
	notification-item.xml \
	notification-watcher.xml

%-client.h: %.xml
	dbus-binding-tool \
		--prefix=_$(notdir $(subst -,_,$(<:.xml=)))_client \
		--mode=glib-client \
		--output=$@ \
		$<

%-server.h: %.xml
	dbus-binding-tool \
		--prefix=_$(notdir $(subst -,_,$(<:.xml=)))_server \
		--mode=glib-server \
		--output=$@ \
		$<

BUILT_SOURCES += \
	$(DBUS_SPECS:.xml=-client.h) \
	$(DBUS_SPECS:.xml=-server.h)

CLEANFILES += $(BUILT_SOURCES)

EXTRA_DIST += $(DBUS_SPECS)

#########################
# GObject Introsepction 
#########################

-include $(INTROSPECTION_MAKEFILE)
INTROSPECTION_GIRS = 
INTROSPECTION_SCANNER_ARGS = \
	--add-include-path=$(srcdir) \
	$(addprefix --c-include=src/, $(introspection_sources)) \
	--symbol-prefix=app \
	--identifier-prefix=App
INTROSPECTION_COMPILER_ARGS = --includedir=$(builddir)

if HAVE_INTROSPECTION

introspection_sources = \
	$(addprefix $(srcdir)/,$(libappindicator_headers))

AppIndicator$(VER)-0.1.gir: libappindicator$(VER).la
AppIndicator_0_1_gir_INCLUDES = \
    GObject-2.0 \
    $(GTKGIR)
AppIndicator_0_1_gir_CFLAGS = $(INDICATOR_CFLAGS) -I$(srcdir) -I$(top_builddir)/src
AppIndicator_0_1_gir_LIBS = libappindicator$(VER).la
AppIndicator_0_1_gir_FILES = $(introspection_sources)

AppIndicator3_0_1_gir_INCLUDES = $(AppIndicator_0_1_gir_INCLUDES)
AppIndicator3_0_1_gir_CFLAGS = $(AppIndicator_0_1_gir_CFLAGS)
AppIndicator3_0_1_gir_LIBS = $(AppIndicator_0_1_gir_LIBS)
AppIndicator3_0_1_gir_FILES = $(AppIndicator_0_1_gir_FILES)

INTROSPECTION_GIRS += AppIndicator$(VER)-0.1.gir

girdir = $(datadir)/gir-1.0
gir_DATA = $(INTROSPECTION_GIRS)

typelibdir = $(libdir)/girepository-1.0
typelib_DATA = $(INTROSPECTION_GIRS:.gir=.typelib)

CLEANFILES += $(gir_DATA) $(typelib_DATA)

endif

#########################
# VAPI Files
#########################

if HAVE_INTROSPECTION

vapidir = $(datadir)/vala/vapi
vapi_DATA = AppIndicator$(VER)-0.1.vapi

AppIndicator$(VER)-0.1.vapi: AppIndicator$(VER)-0.1.gir Makefile.am
	$(VALA_API_GEN) --library=AppIndicator$(VER)-0.1 \
		--pkg $(GTKVAPI) \
		--vapidir=$(top_builddir)/src \
		$<

CLEANFILES += $(vapi_DATA)

endif

