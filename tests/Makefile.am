if USE_GTK3
VER=3
else
VER=
endif


check_PROGRAMS = \
	test-libappindicator \
	test-libappindicator-dbus-client \
	test-libappindicator-dbus-server \
	test-libappindicator-status-client \
	test-libappindicator-status-server \
	test-libappindicator-fallback-watcher \
	test-libappindicator-fallback-item \
	test-simple-app

TESTS =
DISTCLEANFILES = $(TESTS)

EXTRA_DIST = \
	run-xvfb.sh \
	test-libappindicator.desktop

#########################################
##  test-libappindicator
#########################################

test_libappindicator_SOURCES = \
	test-libappindicator.c

test_libappindicator_CFLAGS = \
	$(TESTDEPS_CFLAGS) $(LIBRARY_CFLAGS) \
	-DSRCDIR="\"$(srcdir)\"" \
	-Wall -Werror \
	-I$(top_srcdir)/src

test_libappindicator_LDADD = \
	$(TESTDEPS_LIBS) $(LIBRARY_LIBS) \
	$(top_builddir)/src/libappindicator$(VER).la

#########################################
##  test-libappindicator-dbus-client
#########################################

test_libappindicator_dbus_client_SOURCES = \
	test-defines.h \
	test-libappindicator-dbus-client.c

test_libappindicator_dbus_client_CFLAGS = \
	$(TESTDEPS_CFLAGS) $(LIBRARY_CFLAGS) \
	-Wall -Werror \
	-I$(top_srcdir)/src

test_libappindicator_dbus_client_LDADD = \
	$(TESTDEPS_LIBS) $(LIBRARY_LIBS) \
	$(top_builddir)/src/libappindicator$(VER).la

#########################################
##  test-libappindicator-dbus-server
#########################################

test_libappindicator_dbus_server_SOURCES = \
	test-defines.h \
	test-libappindicator-dbus-server.c

test_libappindicator_dbus_server_CFLAGS = \
	$(TESTDEPS_CFLAGS) $(LIBRARY_CFLAGS) \
	-Wall -Werror \
	-I$(top_srcdir)/src

test_libappindicator_dbus_server_LDADD = \
	$(TESTDEPS_LIBS) $(LIBRARY_LIBS) \
	$(top_builddir)/src/libappindicator$(VER).la

#########################################
##  test-libappindicator-status-client
#########################################

test_libappindicator_status_client_SOURCES = \
	test-defines.h \
	test-libappindicator-status-client.c

test_libappindicator_status_client_CFLAGS = \
	$(TESTDEPS_CFLAGS) $(LIBRARY_CFLAGS) \
	-Wall -Werror \
	-I$(top_srcdir)/src

test_libappindicator_status_client_LDADD = \
	$(TESTDEPS_LIBS) $(LIBRARY_LIBS) \
	$(top_builddir)/src/libappindicator$(VER).la

#########################################
##  test-libappindicator-status-server
#########################################

test_libappindicator_status_server_SOURCES = \
	test-defines.h \
	test-libappindicator-status-server.c

test_libappindicator_status_server_CFLAGS = \
	$(TESTDEPS_CFLAGS) $(LIBRARY_CFLAGS) \
	-Wall -Werror \
	-I$(top_srcdir)/src

test_libappindicator_status_server_LDADD = \
	$(TESTDEPS_LIBS) $(LIBRARY_LIBS) \
	$(top_builddir)/src/libappindicator$(VER).la

#########################################
##  test-libappindicator-fallback
#########################################

test_libappindicator_fallback_watcher_SOURCES = \
	test-libappindicator-fallback-watcher.c

test_libappindicator_fallback_watcher_CFLAGS = \
	$(TESTDEPS_CFLAGS) $(LIBRARY_CFLAGS) \
	-Wall -Werror \
	-I$(top_srcdir)/src

test_libappindicator_fallback_watcher_LDADD = \
	$(TESTDEPS_LIBS) $(LIBRARY_LIBS) \
	$(top_builddir)/src/libappindicator$(VER).la

test_libappindicator_fallback_item_SOURCES = \
	test-libappindicator-fallback-item.c

test_libappindicator_fallback_item_CFLAGS = \
	$(TESTDEPS_CFLAGS) $(LIBRARY_CFLAGS) \
	-Wall -Werror \
	-I$(top_srcdir)/src

test_libappindicator_fallback_item_LDADD = \
	$(TESTDEPS_LIBS) $(LIBRARY_LIBS) \
	$(top_builddir)/src/libappindicator$(VER).la

test-libappindicator-fallback: test-libappindicator-fallback-watcher test-libappindicator-fallback-item Makefile.am
	@echo "#!/bin/bash" > $@
	@echo . $(srcdir)/run-xvfb.sh >> $@
	@echo $(DBUS_RUNNER) --task ./test-libappindicator-fallback-watcher --task-name Watcher --ignore-return --task ./test-libappindicator-fallback-item --task-name Item >> $@
	@chmod +x $@

TESTS += test-libappindicator-fallback

#########################################
##  Actual tests
#########################################

XML_REPORT   = libappindicator-check-results.xml
HTML_REPORT  = libappindicator-check-results.html

libappindicator-tests: libappindicator-tests-gtester Makefile.am
	@echo "#!/bin/bash" > $@
	@echo . $(srcdir)/run-xvfb.sh >> $@
	@echo export UBUNTU_MENUPROXY= >> $@
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
	@echo "#!/bin/bash" > test-libappindicator-dbus
	@echo . $(srcdir)/run-xvfb.sh >> $@
	@echo $(DBUS_RUNNER) --task ./test-libappindicator-dbus-client --task-name Client --task ./test-libappindicator-dbus-server --task-name Server --ignore-return >> test-libappindicator-dbus
	@chmod +x test-libappindicator-dbus

TESTS += test-libappindicator-dbus
XFAIL_TESTS = test-libappindicator-dbus

test-libappindicator-status: test-libappindicator-status-client test-libappindicator-status-server Makefile.am
	@echo "#!/bin/bash" > test-libappindicator-status
	@echo . $(srcdir)/run-xvfb.sh >> $@
	@echo $(DBUS_RUNNER) --task ./test-libappindicator-status-client --task-name Client --task ./test-libappindicator-status-server --task-name Server --ignore-return >> test-libappindicator-status
	@chmod +x test-libappindicator-status

TESTS += test-libappindicator-status

#########################################
##  test-simple-app
#########################################

test_simple_app_SOURCES = \
	test-simple-app.c

test_simple_app_CFLAGS = \
	$(TESTDEPS_CFLAGS) $(LIBRARY_CFLAGS) \
	-Wall -Werror \
	-I$(top_srcdir)/src

test_simple_app_LDADD = \
	$(TESTDEPS_LIBS) $(LIBRARY_LIBS) \
	$(top_builddir)/src/libappindicator$(VER).la

