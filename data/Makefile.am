
dbus_servicesdir = $(DBUSSERVICEDIR)
dbus_services_DATA = indicator-application.service

%.service: %.service.in
	sed -e "s|\@libexecdir\@|$(libexecdir)|" $< > $@

overridedir = $(pkgdatadir)
override_DATA = ordering-override.keyfile

EXTRA_DIST = \
	indicator-application.service.in \
	ordering-override.keyfile

CLEANFILES = \
	indicator-application.service

