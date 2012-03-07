SUBDIRS = . examples

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = appindicator-sharp-0.1.pc

if BUILD_MONO_TEST
TEST = AppIndicator.Test.dll
endif

API = libappindicator-api.xml
MIDDLE_API = libappindicator-api.middle
RAW_API = libappindicator-api.raw
METADATA = libappindicator-api.metadata
ASSEMBLY_NAME = appindicator-sharp
ASSEMBLY_VERSION = 0.2.0.0
ASSEMBLY = $(ASSEMBLY_NAME).dll
POLICY = policy.$(POLICY_VERSION).$(ASSEMBLY_NAME)
POLICY_VERSION = 0.0
DLLPOLICY = $(POLICY).dll
POLICY1 = policy.$(POLICY1_VERSION).$(ASSEMBLY_NAME)
POLICY1_VERSION = 0.1
DLLPOLICY1 = $(POLICY1).dll
WRAPPER_FREE_BINDING_SRC = $(top_srcdir)/src/app-indicator.c
WRAPPER_FREE_BINDING = app-indicator.c

TARGET = \
	$(ASSEMBLY) \
	$(ASSEMBLY).config \
	$(DLLPOLICY) \
	$(POLICY).config \
	$(DLLPOLICY1) \
	$(POLICY1).config

assemblydir = $(libdir)/cli/appindicator-sharp-0.1
assembly_DATA = $(TARGET)

CLEANFILES = \
	$(ASSEMBLY) \
	$(ASSEMBLY).mdb \
	generated-stamp \
	generated/*.cs \
	$(API) \
	$(MIDDLE_API) \
	$(RAW_API) \
	$(DLLPOLICY) \
	$(DLLPOLICY1) \
	$(WRAPPER_FREE_BINDING) \
	$(POLICY).config \
	$(POLICY1).config

if BUILD_MONO_TEST
CLEANFILES += $(TEST) 
endif

DISTCLEANFILES = $(ASSEMBLY).config

if BUILD_MONO_TEST
TEST_SOURCES = TestIndicator.cs
endif

customs = ApplicationIndicator.custom

EXTRA_DIST =					\
	AssemblyInfo.cs \
	$(RAW_API)				\
	$(METADATA)				\
	appindicator-sharp-0.1.pc.in		\
	appindicator-sharp.dll.config.in	\
	app-indicator.sources.xml.in		\
	$(ASSEMBLY_NAME).snk			\
	$(POLICY).config.in \
	$(POLICY1).config.in \
	$(customs)

if BUILD_MONO_TEST
EXTRA_DIST += $(TEST_SOURCES)
endif

GACUTIL_FLAGS=-package $(ASSEMBLY_NAME) -root $(DESTDIR)$(prefix)/lib

references = $(GTK_SHARP_LIBS)

if BUILD_MONO_TEST
test_references = $(GTK_SHARP_LIBS) $(NUNIT_LIBS) $(MONO_NUNIT_LIBS) -r:$(ASSEMBLY)
endif

$(RAW_API): app-indicator.sources.xml $(WRAPPER_FREE_BINDING)
	$(GAPI_PARSER) app-indicator.sources.xml

$(WRAPPER_FREE_BINDING): $(WRAPPER_FREE_BINDING_SRC)
	sed '/signals\[X_NEW_LABEL\] /,+6d' $(WRAPPER_FREE_BINDING_SRC) > $(WRAPPER_FREE_BINDING)

$(MIDDLE_API): $(METADATA) $(RAW_API)
	cp $(RAW_API) $(MIDDLE_API)
	chmod u+w $(MIDDLE_API)
	@if test -n '$(METADATA)'; then							\
		echo "$(GAPI_FIXUP) --api=$(MIDDLE_API) --metadata=$(srcdir)/$(METADATA)"; 	\
		$(GAPI_FIXUP) --api=$(MIDDLE_API) --metadata=$(srcdir)/$(METADATA);		\
	fi

$(API): $(MIDDLE_API) Makefile.am
	sed -e "s|PROP_ID_S|id|" \
		-e "s|PROP_STATUS_S|Status|" \
		-e "s|PROP_STATUS_S|status|" \
		-e "s|PROP_CATEGORY_S|Category|" \
		-e "s|PROP_CATEGORY_S|category|" \
		-e "s|PROP_ICON_NAME_S|icon-name|" \
		-e "s|PROP_ICON_DESC_S|IconDesc|" \
		-e "s|PROP_ICON_DESC_S|icon-desc|" \
		-e "s|PROP_ATTENTION_ICON_NAME_S|AttentionIconName|" \
		-e "s|PROP_ATTENTION_ICON_NAME_S|attention-icon-name|" \
		-e "s|PROP_ATTENTION_ICON_DESC_S|AttentionIconDesc|" \
		-e "s|PROP_ATTENTION_ICON_DESC_S|attention-icon-desc|" \
		-e "s|PROP_ICON_THEME_PATH_S|icon-theme-path|" \
		-e "s|PROP_MENU_S|menu|" \
		-e "s|PROP_CONNECTED_S|connected|" \
		-e "s|PROP_LABEL_S|label|" \
		-e "s|PROP_LABEL_GUIDE_S|label-guide|" \
		-e "s|PROP_ORDERING_INDEX_S|ordering-index|" \
		-e "s|PROP_DBUS_MENU_SERVER_S|DbusMenuServer|" \
		-e "s|PROP_DBUS_MENU_SERVER_S|dbus-menu-server|" \
		-e "s|PROP_TITLE_S|Title|" \
		-e "s|PROP_TITLE_S|title|" \
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
	echo "$(GACUTIL) -i $(ASSEMBLY_NAME).dll $(GACUTIL_FLAGS)";  \
            $(GACUTIL) -i $(ASSEMBLY_NAME).dll $(GACUTIL_FLAGS) || exit 1;
	echo "$(GACUTIL) -i $(DLLPOLICY) $(GACUTIL_FLAGS)";  \
            $(GACUTIL) -i $(DLLPOLICY) $(GACUTIL_FLAGS) || exit 1;

uninstall-local:
	echo "$(GACUTIL) -u $(ASSEMBLY_NAME) $(GACUTIL_FLAGS)"; \
            $(GACUTIL) -u $(ASSEMBLY_NAME) $(GACUTIL_FLAGS) || exit 1;
	echo "$(GACUTIL) -u $(DLLPOLICY) $(GACUTIL_FLAGS)";  \
            $(GACUTIL) -u $(POLICY) $(GACUTIL_FLAGS) || exit 1;

if BUILD_MONO_TEST
$(TEST): $(ASSEMBLY) $(TEST_SOURCES)
	$(CSC) -out:$(TEST) -target:library $(test_references) $(srcdir)/$(TEST_SOURCES)

all: $(TEST)
endif

$(POLICY).config: $(POLICY).config.in Makefile
	sed -e "s|@ASSEMBLY_NAME@|$(ASSEMBLY_NAME)|" \
		-e "s|@ASSEMBLY_VERSION@|$(ASSEMBLY_VERSION)|g" \
		$< > $@

$(DLLPOLICY): $(POLICY).config $(ASSEMBLY_NAME).snk Makefile
	$(AL) -link:$(POLICY).config -out:$(DLLPOLICY) -keyfile:$(srcdir)/$(ASSEMBLY_NAME).snk


$(POLICY1).config: $(POLICY1).config.in Makefile
	sed -e "s|@ASSEMBLY_NAME@|$(ASSEMBLY_NAME)|" \
		-e "s|@ASSEMBLY_VERSION@|$(ASSEMBLY_VERSION)|g" \
		$< > $@

$(DLLPOLICY1): $(POLICY1).config $(ASSEMBLY_NAME).snk Makefile
	$(AL) -link:$(POLICY1).config -out:$(DLLPOLICY1) -keyfile:$(srcdir)/$(ASSEMBLY_NAME).snk
