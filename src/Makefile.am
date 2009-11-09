CLEANFILES = 
DISTCLEANFILES = 
BUILT_SOURCES = 
EXTRA_DIST = 

include $(top_srcdir)/Makefile.am.enum
include $(top_srcdir)/Makefile.am.marshal

##################################
# Indicator
##################################

customlibdir = $(INDICATORDIR)
customlib_LTLIBRARIES = libcustom.la
libcustom_la_SOURCES = \
	dbus-shared.h \
	indicator-custom.c
libcustom_la_CFLAGS = $(INDICATOR_CFLAGS) \
	-Wall \
	-Wl,-Bsymbolic-functions \
	-Wl,-z,defs \
	-Wl,--as-needed \
	-Werror
libcustom_la_LIBADD = $(INDICATOR_LIBS)
libcustom_la_LDFLAGS = -module -avoid-version

##################################
# Service
##################################

libexec_PROGRAMS = indicator-custom-service

indicator_custom_service_SOURCES = \
	custom-service.c \
	custom-service-appstore.h \
	custom-service-appstore.c \
	custom-service-marshal.h \
	custom-service-marshal.c \
	custom-service-server.h \
	custom-service-watcher.h \
	custom-service-watcher.c \
	dbus-properties-client.h \
	dbus-shared.h \
	notification-item-client.h \
	notification-watcher-server.h
indicator_custom_service_CFLAGS = \
	$(INDICATOR_CFLAGS) \
	-Wall -Werror
indicator_custom_service_LDADD = \
	$(INDICATOR_LIBS)

glib_marshal_list = custom-service-marshal.list
glib_marshal_prefix = _custom_service_marshal

##################################
# Library
##################################

glib_enum_h = libcustomindicator/custom-indicator-enum-types.h
glib_enum_c = libcustomindicator/custom-indicator-enum-types.c
glib_enum_headers = $(libcustomindicator_headers)

lib_LTLIBRARIES = \
	libcustomindicator.la

libcustomindicatorincludedir=$(includedir)/libcustomindicator-0.1/libcustomindicator

libcustomindicator_headers = \
	$(srcdir)/libcustomindicator/custom-indicator.h

libcustomindicatorinclude_HEADERS = \
	$(libcustomindicator_headers) \
	$(glib_enum_h)

libcustomindicator_la_SOURCES = \
	$(libcustomindicator_headers) \
	$(glib_enum_c) \
	notification-watcher-client.h \
	notification-item-server.h \
	libcustomindicator/custom-indicator.c

libcustomindicator_la_LDFLAGS = \
	-version-info 0:0:0 \
	-no-undefined \
	-export-symbols-regex "^[^_d].*"

libcustomindicator_la_CFLAGS = \
	$(INDICATOR_CFLAGS) \
	-Wall -Werror

libcustomindicator_la_LIBADD = \
	$(INDICATOR_LIBS)

##################################
# DBus Specs
##################################

DBUS_SPECS = \
	custom-service.xml \
	dbus-properties.xml \
	notification-item.xml \
	notification-watcher.xml

%-client.h: %.xml
	dbus-binding-tool \
		--prefix=_$(subst -,_,$(<:.xml=))_client \
		--mode=glib-client \
		--output=$@ \
		$<

%-server.h: %.xml
	dbus-binding-tool \
		--prefix=_$(subst -,_,$(<:.xml=))_server \
		--mode=glib-server \
		--output=$@ \
		$<

BUILT_SOURCES += \
	$(DBUS_SPECS:.xml=-client.h) \
	$(DBUS_SPECS:.xml=-server.h)

CLEANFILES += $(BUILT_SOURCES)

EXTRA_DIST += $(DBUS_SPECS)
