ASSEMBLY = IndicatorExample.exe
CSFILES = IndicatorExample.cs
CLEANFILES = $(ASSEMBLY)

EXTRA_DIST = $(CSFILES)

references = $(GTK_SHARP_LIBS) -r:$(top_builddir)/bindings/mono/appindicator-sharp.dll

$(ASSEMBLY):
	$(CSC) $(CSFLAGS) -out:$(ASSEMBLY) -target:exe $(references) $(CSFILES)

all: $(ASSEMBLY)