
##################################
# Indicator
##################################

customlibdir = $(INDICATORDIR)
customlib_LTLIBRARIES = libcustom.la
libcustom_la_SOURCES = \
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
	notification-item-client.h \
	notification-watcher-server.h
indicator_custom_service_CFLAGS = \
	$(INDICATOR_CFLAGS) \
	-Wall -Werror
indicator_custom_service_LDADD = \
	$(INDICATOR_LIBS)

##################################
# DBus Specs
##################################

DBUS_SPECS = \
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

BUILT_SOURCES = \
	$(DBUS_SPECS:.xml=-client.h) \
	$(DBUS_SPECS:.xml=-server.h)

DIST_CLEAN = $(BUILT_SOURCES)
