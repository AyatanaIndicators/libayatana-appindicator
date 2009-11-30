
dbus_servicesdir = $(DBUSSERVICEDIR)
dbus_services_DATA = indicator-application.service

%.service: %.service.in
	sed -e "s|\@libexecdir\@|$(libexecdir)|" $< > $@

EXTRA_DIST = indicator-application.service.in

CLEANFILES = indicator-application.service

