
AC_INIT(indicator-application, 0.0.4, ted@canonical.com)
AC_COPYRIGHT([Copyright 2009 Canonical])

AC_PREREQ(2.53)

AM_CONFIG_HEADER(config.h)
AM_INIT_AUTOMAKE(indicator-application, 0.0.4)

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
INDICATOR_REQUIRED_VERSION=0.3.0
DBUSMENUGTK_REQUIRED_VERSION=0.1.1

PKG_CHECK_MODULES(INDICATOR, gtk+-2.0 >= $GTK_REQUIRED_VERSION
                             indicator >= $INDICATOR_REQUIRED_VERSION
                             dbusmenu-gtk >= $DBUSMENUGTK_REQUIRED_VERSION)

AC_SUBST(INDICATOR_CFLAGS)
AC_SUBST(INDICATOR_LIBS)

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
# Files
###########################

AC_OUTPUT([
Makefile
src/Makefile
src/libappindicator/appindicator-0.1.pc
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
