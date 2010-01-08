CLEANFILES = 
DISTCLEANFILES = 
BUILT_SOURCES = 
EXTRA_DIST = libappindicator/appindicator-0.1.pc.in

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
	-Werror
libapplication_la_LIBADD = $(INDICATOR_LIBS)
libapplication_la_LDFLAGS = -module -avoid-version

##################################
# Service
##################################

libexec_PROGRAMS = indicator-application-service

indicator_application_service_SOURCES = \
	application-service.c \
	application-service-appstore.h \
	application-service-appstore.c \
	application-service-marshal.h \
	application-service-marshal.c \
	application-service-server.h \
	application-service-watcher.h \
	application-service-watcher.c \
	dbus-properties-client.h \
	dbus-shared.h \
	notification-item-client.h \
	notification-watcher-server.h
indicator_application_service_CFLAGS = \
	$(INDICATOR_CFLAGS) \
	-Wall -Werror
indicator_application_service_LDADD = \
	$(INDICATOR_LIBS)

glib_marshal_list = application-service-marshal.list
glib_marshal_prefix = _application_service_marshal

##################################
# Library
##################################

pkgconfig_DATA = libappindicator/appindicator-0.1.pc
pkgconfigdir = $(libdir)/pkgconfig

glib_enum_h = libappindicator/app-indicator-enum-types.h
glib_enum_c = libappindicator/app-indicator-enum-types.c
glib_enum_headers = $(libappindicator_headers)

lib_LTLIBRARIES = \
	libappindicator.la

libappindicatorincludedir=$(includedir)/libappindicator-0.1/libappindicator

libappindicator_headers = \
	$(srcdir)/libappindicator/app-indicator.h

libappindicatorinclude_HEADERS = \
	$(libappindicator_headers) \
	$(glib_enum_h)

libappindicator_la_SOURCES = \
	$(libappindicator_headers) \
	$(glib_enum_c) \
	notification-watcher-client.h \
	notification-item-server.h \
	libappindicator/app-indicator.c

libappindicator_la_LDFLAGS = \
	-version-info 0:0:0 \
	-no-undefined \
	-export-symbols-regex "^[^_d].*"

libappindicator_la_CFLAGS = \
	$(INDICATOR_CFLAGS) \
	-Wall -Werror

libappindicator_la_LIBADD = \
	$(INDICATOR_LIBS)

##################################
# DBus Specs
##################################

DBUS_SPECS = \
	application-service.xml \
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
