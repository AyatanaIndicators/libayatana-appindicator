ASSEMBLY = AyatanaIndicatorExample.exe
CSFILES = AyatanaIndicatorExample.cs
CLEANFILES = $(ASSEMBLY)
DISTCLEANFILES = Makefile.in

EXTRA_DIST = $(CSFILES)

references = $(GTK_SHARP_LIBS) -r:$(top_builddir)/bindings/mono/ayatana-appindicator-sharp.dll

$(ASSEMBLY): $(CSFILES) Makefile.am
	$(CSC) $(CSFLAGS) -out:$(ASSEMBLY) -target:exe $(references) $(srcdir)/$(CSFILES)

all: $(ASSEMBLY)
