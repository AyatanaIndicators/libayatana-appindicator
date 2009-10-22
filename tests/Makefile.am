
check_PROGRAMS = \
	test-libcustomindicator \
	test-libcustomindicator-dbus-client \
	test-libcustomindicator-dbus-server

#########################################
##  test-libcustomindicator
#########################################

test_libcustomindicator_SOURCES = \
	test-libcustomindicator.c

test_libcustomindicator_CFLAGS = \
	$(INDICATOR_CFLAGS) \
	-Wall -Werror \
	-I$(top_srcdir)/src

test_libcustomindicator_LDADD = \
	$(INDICATOR_LIBS) \
	$(top_builddir)/src/libcustomindicator.la

#########################################
##  test-libcustomindicator-dbus-client
#########################################

test_libcustomindicator_dbus_client_SOURCES = \
	test-defines.h \
	test-libcustomindicator-dbus-client.c

test_libcustomindicator_dbus_client_CFLAGS = \
	$(INDICATOR_CFLAGS) \
	-Wall -Werror \
	-I$(top_srcdir)/src

test_libcustomindicator_dbus_client_LDADD = \
	$(INDICATOR_LIBS) \
	$(top_builddir)/src/libcustomindicator.la

#########################################
##  test-libcustomindicator-dbus-server
#########################################

test_libcustomindicator_dbus_server_SOURCES = \
	test-defines.h \
	test-libcustomindicator-dbus-server.c

test_libcustomindicator_dbus_server_CFLAGS = \
	$(INDICATOR_CFLAGS) \
	-Wall -Werror \
	-I$(top_srcdir)/src

test_libcustomindicator_dbus_server_LDADD = \
	$(INDICATOR_LIBS) \
	$(top_builddir)/src/libcustomindicator.la

#########################################
##  Actual tests
#########################################

XML_REPORT   = libcustomindicator-check-results.xml
HTML_REPORT  = libcustomindicator-check-results.html

libcustomindicator-tester: test-libcustomindicator
	gtester -k --verbose -o=$(XML_REPORT) ./test-libcustomindicator

check-local: libcustomindicator-tester

DISTCLEANFILES = $(XML_REPORT) $(HTML_REPORT)

