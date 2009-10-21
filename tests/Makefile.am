
check_PROGRAMS = \
	test-libcustomindicator

test_libcustomindicator_SOURCES = \
	test-libcustomindicator.c

test_libcustomindicator_CFLAGS = \
	$(INDICATOR_CFLAGS) \
	-Wall -Werror \
	-I$(top_srcdir)/src

test_libcustomindicator_LDADD = \
	$(INDICATOR_LIBS) \
	$(top_builddir)/src/libcustomindicator.la

XML_REPORT   = libcustomindicator-check-results.xml
HTML_REPORT  = libcustomindicator-check-results.html

libcustomindicator-tester: test-libcustomindicator
	gtester -o=$(XML_REPORT) ./test-libcustomindicator

check-local: libcustomindicator-tester

DISTCLEANFILES = $(XML_REPORT) $(HTML_REPORT)

