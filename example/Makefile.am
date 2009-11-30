
check_PROGRAMS = \
	simple-client

#########################################
##  simple-client
#########################################

simple_client_SOURCES = \
	simple-client.c

simple_client_CFLAGS = \
	$(INDICATOR_CFLAGS) \
	-Wall -Werror \
	-I$(top_srcdir)/src

simple_client_LDADD = \
	$(INDICATOR_LIBS) \
	$(top_builddir)/src/libappindicator.la
