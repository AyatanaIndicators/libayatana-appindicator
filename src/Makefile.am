
libexec_PROGRAMS = indicator-custom-service

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

indicator_custom_service_SOURCES = \
	custom-service.c
indicator_custom_service_CFLAGS = \
	$(INDICATOR_CFLAGS) \
	-Wall -Werror
indicator_custom_service_LDADD = \
	$(INDICATOR_LIBS)
