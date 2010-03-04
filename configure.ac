
AC_INIT(indicator-application, 0.0.15, ted@canonical.com)
AC_COPYRIGHT([Copyright 2009, 2010 Canonical])

AC_PREREQ(2.53)

AM_CONFIG_HEADER(config.h)
AM_INIT_AUTOMAKE(indicator-application, 0.0.15)

AM_MAINTAINER_MODE

AC_ISC_POSIX
AC_PROG_CC
AM_PROG_CC_C_O
AC_STDC_HEADERS
AC_PROG_LIBTOOL

AC_SUBST(VERSION)
AC_CONFIG_MACRO_DIR([m4])

m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

AC_PATH_PROG([GLIB_MKENUMS], [glib-mkenums])
AC_PATH_PROG([GLIB_GENMARSHAL], [glib-genmarshal])

PKG_PROG_PKG_CONFIG

###########################
# GTK Doc
###########################

GTK_DOC_CHECK([1.9])
AC_CONFIG_MACRO_DIR(m4)


###########################
# Dependencies
###########################

GTK_REQUIRED_VERSION=2.12
INDICATOR_REQUIRED_VERSION=0.3.1
DBUSMENUGTK_REQUIRED_VERSION=0.2.2
JSON_GLIB_REQUIRED_VERSION=0.7.6
DBUS_GLIB_REQUIRED_VERSION=0.82

PKG_CHECK_MODULES(INDICATOR, gtk+-2.0 >= $GTK_REQUIRED_VERSION
                             indicator >= $INDICATOR_REQUIRED_VERSION
                             json-glib-1.0 >= $JSON_GLIB_REQUIRED_VERSION
                             dbus-glib-1 >= $DBUS_GLIB_REQUIRED_VERSION
                             dbusmenu-gtk >= $DBUSMENUGTK_REQUIRED_VERSION)

AC_SUBST(INDICATOR_CFLAGS)
AC_SUBST(INDICATOR_LIBS)

###########################
# Check for Mono support
###########################

MONO_REQUIRED_VERSION=1.0
PKG_CHECK_MODULES(MONO_DEPENDENCY, mono >= $MONO_REQUIRED_VERSION, has_mono=true, has_mono=false)

AC_PATH_PROG(AL, al, no)
if test "x$AL" = "xno" ; then
   AC_MSG_ERROR([No al tool found.  You need to install Mono.])
fi

if test "x$has_mono" = "xtrue" ; then
   GENERATED_SOURCES=generated/*.cs
   AC_PATH_PROG(RUNTIME, mono, no)

   if test "x$RUNTIME" != "no" ; then
      RUNTIME=mono
   fi

   AC_PATH_PROG(CSC, gmcs, no)
   LIB_PREFIX=.so
   LIB_SUFFIX=
fi

if test "x$CSC" = "xno" ; then
   AC_MSG_ERROR([No Mono compiler found.])
fi

AC_PATH_PROG(GACUTIL, gacutil, no)
if test "x$GACUTIL" = "xno" ; then
   AC_MSG_ERROR([No gacutil tool found])
fi

AC_SUBST(RUNTIME)
AC_SUBST(CSC)
AC_SUBST(GACUTIL)
AC_SUBST(LIB_PREFIX)
AC_SUBST(LIB_SUFFIX)
AC_SUBST(GENERATED_SOURCES)

GLIB_SHARP_REQ_VERSION=2.12.1
GTK_SHARP_REQ_VERSION=2.12
PKG_CHECK_MODULES(GTK_SHARP, gtk-sharp-2.0 >= $GTK_SHARP_REQ_VERSION)
AC_SUBST(GTK_SHARP_LIBS)
AC_SUBST(GTK_SHARP_CFLAGS)

PKG_CHECK_MODULES(GAPI, gapi-2.0 >= $GLIB_SHARP_REQ_VERSION)
AC_PATH_PROG(GAPI_PARSER, gapi2-parser, no)
AC_SUBST(GAPI_PARSER)
AC_PATH_PROG(GAPI_CODEGEN, gapi2-codegen, no)
AC_SUBST(GAPI_CODEGEN)
AC_PATH_PROG(GAPI_FIXUP, gapi2-fixup, no)
AC_SUBST(GAPI_FIXUP)

PKG_CHECK_MODULES(NUNIT, nunit >= 2.4.7)
AC_SUBST(NUNIT_LIBS)

###########################
# Check to see if we're local
###########################

with_localinstall="no"
AC_ARG_ENABLE(localinstall, AS_HELP_STRING([--enable-localinstall], [install all of the files localy instead of system directories (for distcheck)]), with_localinstall=$enableval, with_localinstall=no)

###########################
# Indicator Info
###########################

if test "x$with_localinstall" = "xyes"; then
	INDICATORDIR="${libdir}/indicators/2/"
	INDICATORICONSDIR="${datadir}/libindicate/icons/"
else
	INDICATORDIR=`$PKG_CONFIG --variable=indicatordir indicator`
	INDICATORICONSDIR=`$PKG_CONFIG --variable=iconsdir indicator`
fi
AC_SUBST(INDICATORDIR)
AC_SUBST(INDICATORICONSDIR)

###########################
# DBus Service Info
###########################

if test "x$with_localinstall" = "xyes"; then
	DBUSSERVICEDIR="${datadir}/dbus-1/services/"
else
	DBUSSERVICEDIR=`$PKG_CONFIG --variable=session_bus_services_dir dbus-1`
fi
AC_SUBST(DBUSSERVICEDIR)

###########################
# Python
###########################

PYGTK_REQUIRED=2.14.0
PYGOBJECT_REQUIRED=0.22

AM_PATH_PYTHON(2.3.5)
AM_CHECK_PYTHON_HEADERS(,[AC_MSG_ERROR(could not find Python headers)])

PKG_CHECK_MODULES(APPINDICATOR_PYTHON,
                  [
					pygtk-2.0 >= $PYGTK_REQUIRED
					gtk+-2.0 >= $GTK_REQUIRED_VERSION
					pygobject-2.0 >= $PYGOBJECT_REQUIRED
                  ])
AC_SUBST(APPINDICATOR_PYTHON_CFLAGS)
AC_SUBST(APPINDICATOR_PYTHON_LIBS)

AC_MSG_CHECKING(for pygtk defs)
PYGTK_DEFSDIR=`$PKG_CONFIG --variable=defsdir pygtk-2.0`
AC_SUBST(PYGTK_DEFSDIR)
AC_MSG_RESULT($PYGTK_DEFSDIR)

AC_MSG_CHECKING(for pygtk codegen)
PYGTK_CODEGEN="$PYTHON `$PKG_CONFIG --variable=codegendir pygtk-2.0`/codegen.py"
AC_SUBST(PYGTK_CODEGEN)
AC_MSG_RESULT($PYGTK_CODEGEN)

###########################
# Files
###########################

AC_OUTPUT([
Makefile
src/Makefile
src/libappindicator/appindicator-0.1.pc
bindings/Makefile
bindings/mono/Makefile
bindings/mono/appindicator-sharp.dll.config
bindings/mono/appindicator-sharp-0.1.pc
bindings/mono/examples/Makefile
bindings/mono/examples/indicator-example
bindings/python/Makefile
data/Makefile
tests/Makefile
example/Makefile
docs/Makefile
docs/reference/Makefile
docs/reference/version.xml
docs/reference/libappindicator-docs.sgml
])

###########################
# Results
###########################

AC_MSG_NOTICE([

Application Indicator Configuration:

	Prefix:        $prefix
	Indicator Dir: $INDICATORDIR
])
