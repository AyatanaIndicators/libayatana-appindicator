SUBDIRS = . examples


#########################
# VAPI Files
#########################

if HAVE_INTROSPECTION

if USE_GTK3
GTKVAPI = gtk+-3.0
else
GTKVAPI = gtk+-2.0
endif

vapidir = $(datadir)/vala/vapi
vapiprefix = appindicator$(VER)-0.1
vapi_DATA = $(vapiprefix).vapi $(vapiprefix).deps
DEPS = $(GTKVAPI)

$(vapiprefix).deps:
	echo $(DEPS) > $@

$(vapiprefix).vapi: $(top_builddir)/src/AppIndicator$(VER)-0.1.gir $(vapiprefix).deps
	$(VALA_API_GEN) --library=$(vapiprefix) \
		--pkg $(GTKVAPI) \
		$<
	

CLEANFILES = $(vapi_DATA)

endif
