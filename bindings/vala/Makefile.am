if HAVE_INTROSPECTION
if HAVE_VAPIGEN

SUBDIRS = . examples

#########################
# VAPI Files
#########################

if USE_GTK3
GTKVAPI = gtk+-3.0
VER=3
else
GTKVAPI = gtk+-2.0
VER=
endif

vapidir = $(datadir)/vala/vapi
vapiprefix = ayatana-appindicator$(VER)-0.1
vapi_DATA = $(vapiprefix).vapi $(vapiprefix).deps
DEPS = $(GTKVAPI)

$(vapiprefix).deps:
	echo $(DEPS) > $@

$(vapiprefix).vapi: $(top_builddir)/src/AyatanaAppIndicator$(VER)-0.1.gir \
                    $(top_builddir)/src/AyatanaAppIndicator$(VER)-0.1.metadata \
                    ayatana-appindicator-0.1-custom.vala \
                    $(vapiprefix).deps
	$(VALA_API_GEN) --library=$(vapiprefix) --girdir=$(srcdir)/src \
	$< $(srcdir)/ayatana-appindicator-0.1-custom.vala
	

CLEANFILES = $(vapi_DATA)
DISTCLEANFILES = Makefile.in

endif
endif

EXTRA_DIST = \
	ayatana-appindicator-0.1-custom.vala
