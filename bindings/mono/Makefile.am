SUBDIRS = . examples

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = appindicator-sharp-0.1.pc

TEST = AppIndicator.Test.dll

API = libappindicator-api.xml
MIDDLE_API = libappindicator-api.middle
RAW_API = libappindicator-api.raw
METADATA = libappindicator-api.metadata
ASSEMBLY_NAME = appindicator-sharp
ASSEMBLY = appindicator-sharp.dll
TARGET = $(ASSEMBLY) $(ASSEMBLY).config
assemblydir = $(libdir)/cli/appindicator-sharp-0.1
assembly_DATA = $(TARGET)
CLEANFILES = $(ASSEMBLY) $(ASSEMBLY).mdb generated-stamp generated/*.cs $(API) $(MIDDLE_API) $(RAW_API) $(TEST)
DISTCLEANFILES = $(ASSEMBLY).config
TEST_SOURCES = TestIndicator.cs
customs = ApplicationIndicator.custom
EXTRA_DIST =					\
	AssemblyInfo.cs \
	$(RAW_API)				\
	$(METADATA)				\
	appindicator-sharp-0.1.pc.in		\
	appindicator-sharp.dll.config.in	\
	app-indicator.sources.xml		\
	$(ASSEMBLY_NAME).snk			\
	$(customs)				\
	$(TEST_SOURCES)

GACUTIL_FLAGS="-package $(ASSEMBLY_NAME) -root $(DESTDIR)$(prefix)/lib"

references = $(GTK_SHARP_LIBS)
test_references = $(GTK_SHARP_LIBS) $(NUNIT_LIBS) -r:$(ASSEMBLY)

$(RAW_API): app-indicator.sources.xml
	$(GAPI_PARSER) app-indicator.sources.xml

$(MIDDLE_API): $(METADATA) $(RAW_API)
	cp $(srcdir)/$(RAW_API) $(MIDDLE_API)
	chmod u+w $(MIDDLE_API)
	@if test -n '$(METADATA)'; then							\
		echo "$(GAPI_FIXUP) --api=$(MIDDLE_API) --metadata=$(srcdir)/$(METADATA)"; 	\
		$(GAPI_FIXUP) --api=$(MIDDLE_API) --metadata=$(srcdir)/$(METADATA);		\
	fi

$(API): $(MIDDLE_API) Makefile.am
	sed -e "s|PROP_ID_S|id|" \
		-e "s|PROP_STATUS_S|status|" \
		-e "s|PROP_CATEGORY_S|category|" \
		-e "s|PROP_ICON_NAME_S|icon-name|" \
		-e "s|PROP_ATTENTION_ICON_NAME_S|attention-icon-name|" \
		-e "s|PROP_ICON_THEME_PATH_S|icon-theme-path|" \
		-e "s|PROP_MENU_S|menu|" \
		-e "s|PROP_CONNECTED_S|connected|" \
		$< > $@

api_includes = $(GTK_SHARP_CFLAGS)

build_customs = $(addprefix $(srcdir)/, $(customs))

generated-stamp: $(API) $(build_customs)
	rm -f generated/* &&					\
	$(GAPI_CODEGEN) --generate $(API) $(api_includes)	\
	--customdir=$(srcdir)					\
	--outdir=generated --assembly-name=$(ASSEMBLY_NAME)	\
	&& touch generated-stamp

$(ASSEMBLY): generated-stamp $(srcdir)/AssemblyInfo.cs
	@rm -f $(ASSEMBLY).mdb
	$(CSC) $(CSFLAGS) -keyfile:$(srcdir)/$(ASSEMBLY_NAME).snk -nowarn:0169,0612,0618 -unsafe -out:$(ASSEMBLY) -target:library $(references) $(builddir)/$(GENERATED_SOURCES) $(srcdir)/AssemblyInfo.cs

install-data-local:
	echo "$(GACUTIL) -i $(ASSEMBLY_NAME).dll  -package $(ASSEMBLY_NAME) -root $(DESTDIR)$(prefix)/lib";  \
            $(GACUTIL) -i $(ASSEMBLY_NAME).dll  -package $(ASSEMBLY_NAME) -root $(DESTDIR)$(prefix)/lib || exit 1;

uninstall-local:
	echo "$(GACUTIL) -u $(ASSEMBLY_NAME) -package $(ASSEMBLY_NAME) -root $(DESTDIR)$(prefix)/lib"; \
            $(GACUTIL) -u $(ASSEMBLY_NAME) -package $(ASSEMBLY_NAME) -root $(DESTDIR)$(prefix)/lib || exit 1;

$(TEST): $(ASSEMBLY) $(TEST_SOURCES)
	$(CSC) -out:$(TEST) -target:library $(test_references) $(srcdir)/$(TEST_SOURCES)

all: $(TEST)
