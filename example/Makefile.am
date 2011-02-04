if USE_GTK3
VER=3
else
VER=
endif

CLEANFILES =

check_PROGRAMS = \
	simple-client \
	simple-client-vala

#########################################
##  simple-client
#########################################

simple_client_SOURCES = \
	simple-client.c

simple_client_CFLAGS = \
	$(LIBRARY_CFLAGS) \
	-DLOCAL_ICON="\"$(abs_srcdir)/simple-client-test-icon.png\"" \
	-Wall -Werror \
	-I$(top_srcdir)/src

simple_client_LDADD = \
	$(LIBRARY_LIBS) \
	$(top_builddir)/src/libappindicator$(VER).la

EXTRA_DIST = \
	simple-client-test-icon.png

#########################################
##  simple-client-vala
#########################################

VALAFLAGS = \
	--pkg appindicator$(VER)-0.1 \
	--vapidir=$(top_builddir)/bindings/vala

BUILT_SOURCES = simple-client-vala.c
simple-client-vala.c: $(srcdir)/simple-client-vala.vala
	$(VALAC) $(VALAFLAGS) -C $< -o $@
	$(SED) -i "s|#include\s<libappindicator/app-indicator.h>||g" $@

simple_client_vala_SOURCES = simple-client-vala.c
simple_client_vala_CFLAGS = \
	$(simple_client_CFLAGS) \
	-include $(top_srcdir)/src/app-indicator.h

simple_client_vala_LDADD = $(simple_client_LDADD)

CLEANFILES += *.stamp *-vala.c
