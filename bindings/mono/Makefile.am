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
assemblydir = $(libdir)/appindicator-sharp
assembly_DATA = $(TARGET)
CLEANFILES = $(ASSEMBLY) $(ASSEMBLY).mdb generated-stamp generated/*.cs $(API) $(RAW_API) $(TEST)
DISTCLEANFILES = $(ASSEMBLY).config
EXTRA_DIST = \
	$(RAW_API) \
	$(METADATA) \
	appindicator-sharp-0.1.pc.in \
	appindicator-sharp.dll.config.in \
	app-indicator.sources.xml \
	$(TEST_SOURCES)

TEST_SOURCES = TestIndicator.cs

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
	$(CSC) $(CSFLAGS) -nowarn:0169,0612,0618 -unsafe -out:$(ASSEMBLY) -target:library $(references) $(builddir)/$(GENERATED_SOURCES)

$(TEST): $(ASSEMBLY) $(TEST_SOURCES)
	$(CSC) -out:$(TEST) -target:library $(test_references) $(srcdir)/$(TEST_SOURCES)

all: $(TEST)
