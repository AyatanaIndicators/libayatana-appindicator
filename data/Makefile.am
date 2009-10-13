
dbus_servicesdir = $(DBUSSERVICEDIR)
dbus_services_DATA = indicator-custom.service

%.service: %.service.in
	sed -e "s|\@libexecdir\@|$(libexecdir)|" $< > $@

EXTRA_DIST = indicator-custom.service.in

CLEANFILES = indicator-custom.service

