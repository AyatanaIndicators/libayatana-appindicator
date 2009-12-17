SUBDIRS = . examples

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = appindicator-sharp-0.1.pc

TEST = AppIndicator.Test.dll

API = libappindicator-api.xml
RAW_API = libappindicator-api.raw
METADATA = libappindicator-api.metadata
ASSEMBLY_NAME = appindicator-sharp
ASSEMBLY = appindicator-sharp.dll
TARGET = $(ASSEMBLY) $(ASSEMBLY).config
assemblydir = $(libdir)/appindicator-sharp-0.1
assembly_DATA = $(TARGET)
CLEANFILES = $(ASSEMBLY) $(ASSEMBLY).mdb generated-stamp generated/*.cs $(API) $(RAW_API) $(TEST)
DISTCLEANFILES = $(ASSEMBLY).config
TEST_SOURCES = TestIndicator.cs
EXTRA_DIST = \
	$(RAW_API) \
	$(METADATA) \
	appindicator-sharp-0.1.pc.in \
	appindicator-sharp.dll.config.in \
	app-indicator.sources.xml \
	$(ASSEMBLY_NAME).snk \
	$(TEST_SOURCES)

GACUTIL_FLAGS="-package $(ASSEMBLY_NAME) -root $(DESTDIR)$(prefix)/lib"

references = $(GTK_SHARP_LIBS)
test_references = $(GTK_SHARP_LIBS) $(NUNIT_LIBS) -r:$(ASSEMBLY)

$(RAW_API): app-indicator.sources.xml
	$(GAPI_PARSER) app-indicator.sources.xml

$(API): $(METADATA) $(RAW_API)
	cp $(srcdir)/$(RAW_API) $(API)
	chmod u+w $(API)
	@if test -n '$(METADATA)'; then							\
		echo "$(GAPI_FIXUP) --api=$(API) --metadata=$(srcdir)/$(METADATA)"; 	\
		$(GAPI_FIXUP) --api=$(API) --metadata=$(srcdir)/$(METADATA);		\
	fi

api_includes = $(GTK_SHARP_CFLAGS)

generated-stamp: $(API)
	rm -f generated/* &&					\
	$(GAPI_CODEGEN) --generate $(API) $(api_includes)	\
	--outdir=generated --assembly-name=$(ASSEMBLY_NAME)	\
	&& touch generated-stamp

$(ASSEMBLY): generated-stamp
	@rm -f $(ASSEMBLY).mdb
	$(CSC) $(CSFLAGS) -keyfile:$(srcdir)/$(ASSEMBLY_NAME).snk -nowarn:0169,0612,0618 -unsafe -out:$(ASSEMBLY) -target:library $(references) $(builddir)/$(GENERATED_SOURCES)

install-data-local:
	echo "$(GACUTIL) -i $(ASSEMBLY_NAME).dll  -package $(ASSEMBLY_NAME) -root $(DESTDIR)$(prefix)/lib";  \
            $(GACUTIL) -i $(ASSEMBLY_NAME).dll  -package $(ASSEMBLY_NAME) -root $(DESTDIR)$(prefix)/lib || exit 1;

uninstall-local:
	echo "$(GACUTIL) -u $(ASSEMBLY_NAME) -package $(ASSEMBLY_NAME) -root $(DESTDIR)$(prefix)/lib"; \
            $(GACUTIL) -u $(ASSEMBLY_NAME) -package $(ASSEMBLY_NAME) -root $(DESTDIR)$(prefix)/lib || exit 1;

$(TEST): $(ASSEMBLY) $(TEST_SOURCES)
	$(CSC) -out:$(TEST) -target:library $(test_references) $(srcdir)/$(TEST_SOURCES)

all: $(TEST)
