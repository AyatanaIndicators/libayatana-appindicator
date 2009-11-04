
check_PROGRAMS = \
	test-libcustomindicator \
	test-libcustomindicator-dbus-client \
	test-libcustomindicator-dbus-server

TESTS =
DISTCLEANFILES = $(TESTS)

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

libcustomindicator-tests: libcustomindicator-tests-gtester Makefile.am
	@echo "#!/bin/sh" > $@
	@echo $(DBUS_RUNNER) --task ./libcustomindicator-tests-gtester >> $@
	@chmod +x $@

libcustomindicator-tests-gtester: test-libcustomindicator Makefile.am
	@echo "#!/bin/sh" > $@
	@echo gtester -k --verbose -o=$(XML_REPORT) ./test-libcustomindicator >> $@
	@chmod +x $@

TESTS += libcustomindicator-tests libcustomindicator-tests-gtester
DISTCLEANFILES += $(XML_REPORT) $(HTML_REPORT)


DBUS_RUNNER=dbus-test-runner --dbus-config /usr/share/dbus-test-runner/session.conf

test-libcustomindicator-dbus: test-libcustomindicator-dbus-client test-libcustomindicator-dbus-server Makefile.am
	@echo "#!/bin/sh" > test-libcustomindicator-dbus
	@echo $(DBUS_RUNNER) --task ./test-libcustomindicator-dbus-client --task-name Client --task ./test-libcustomindicator-dbus-server --task-name Server --ignore-return >> test-libcustomindicator-dbus
	@chmod +x test-libcustomindicator-dbus

TESTS += test-libcustomindicator-dbus

