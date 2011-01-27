if USE_GTK3
VER=3
else
VER=
endif


check_PROGRAMS = \
	simple-client

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
