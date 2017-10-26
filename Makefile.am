ACLOCAL_AMFLAGS = -I m4 ${ACLOCAL_FLAGS}

SUBDIRS = \
	  src		\
	  bindings	\
	  docs

# FIXME: Disabled, vala error
#	  example	

if WANT_TESTS
SUBDIRS += tests
endif

EXTRA_DIST = \
	m4/introspection.m4 \
	gtk-doc.make \
	omf.make \
	xmldocs.make \
	autogen.sh \
	COPYING.LGPL.2.1 \
	ChangeLog

DISTCLEANFILES = \
	INSTALL \
	Makefile.in \
	aclocal.m4 \
	ar-lib \
	autom4te.cache/ \
	compile \
	config.guess \
	config.h.in \
	config.h.in~ \
	config.sub \
	configure \
	depcomp \
	gtk-doc.make \
	install-sh \
	ltmain.sh \
	m4/gtk-doc.m4 \
	m4/libtool.m4 \
	m4/ltoptions.m4 \
	m4/ltsugar.m4 \
	m4/ltversion.m4 \
	m4/lt~obsolete.m4 \
	missing \
	omf.make \
	py-compile \
	test-driver \
	xmldocs.make

include $(top_srcdir)/Makefile.am.coverage
