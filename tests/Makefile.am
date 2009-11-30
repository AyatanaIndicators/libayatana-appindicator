
check_PROGRAMS = \
	test-libappindicator \
	test-libappindicator-dbus-client \
	test-libappindicator-dbus-server \
	test-simple-app

TESTS =
DISTCLEANFILES = $(TESTS)

#########################################
##  test-libappindicator
#########################################

test_libappindicator_SOURCES = \
	test-libappindicator.c

test_libappindicator_CFLAGS = \
	$(INDICATOR_CFLAGS) \
	-Wall -Werror \
	-I$(top_srcdir)/src

test_libappindicator_LDADD = \
	$(INDICATOR_LIBS) \
	$(top_builddir)/src/libappindicator.la

#########################################
##  test-libappindicator-dbus-client
#########################################

test_libappindicator_dbus_client_SOURCES = \
	test-defines.h \
	test-libappindicator-dbus-client.c

test_libappindicator_dbus_client_CFLAGS = \
	$(INDICATOR_CFLAGS) \
	-Wall -Werror \
	-I$(top_srcdir)/src

test_libappindicator_dbus_client_LDADD = \
	$(INDICATOR_LIBS) \
	$(top_builddir)/src/libappindicator.la

#########################################
##  test-libappindicator-dbus-server
#########################################

test_libappindicator_dbus_server_SOURCES = \
	test-defines.h \
	test-libappindicator-dbus-server.c

test_libappindicator_dbus_server_CFLAGS = \
	$(INDICATOR_CFLAGS) \
	-Wall -Werror \
	-I$(top_srcdir)/src

test_libappindicator_dbus_server_LDADD = \
	$(INDICATOR_LIBS) \
	$(top_builddir)/src/libappindicator.la

#########################################
##  Actual tests
#########################################

XML_REPORT   = libappindicator-check-results.xml
HTML_REPORT  = libappindicator-check-results.html

libappindicator-tests: libappindicator-tests-gtester Makefile.am
	@echo "#!/bin/sh" > $@
	@echo $(DBUS_RUNNER) --task ./libappindicator-tests-gtester >> $@
	@chmod +x $@

libappindicator-tests-gtester: test-libappindicator Makefile.am
	@echo "#!/bin/sh" > $@
	@echo gtester -k --verbose -o=$(XML_REPORT) ./test-libappindicator >> $@
	@chmod +x $@

TESTS += libappindicator-tests
DISTCLEANFILES += $(XML_REPORT) $(HTML_REPORT) libappindicator-tests-gtester


DBUS_RUNNER=dbus-test-runner --dbus-config /usr/share/dbus-test-runner/session.conf

test-libappindicator-dbus: test-libappindicator-dbus-client test-libappindicator-dbus-server Makefile.am
	@echo "#!/bin/sh" > test-libappindicator-dbus
	@echo $(DBUS_RUNNER) --task ./test-libappindicator-dbus-client --task-name Client --task ./test-libappindicator-dbus-server --task-name Server --ignore-return >> test-libappindicator-dbus
	@chmod +x test-libappindicator-dbus

TESTS += test-libappindicator-dbus

#########################################
##  test-simple-app
#########################################

test_simple_app_SOURCES = \
	test-simple-app.c

test_simple_app_CFLAGS = \
	$(INDICATOR_CFLAGS) \
	-Wall -Werror \
	-I$(top_srcdir)/src

test_simple_app_LDADD = \
	$(INDICATOR_LIBS) \
	$(top_builddir)/src/libappindicator.la

