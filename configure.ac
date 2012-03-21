AC_INIT([libappindicator],
        [0.4.92],
        [http://bugs.launchpad.net/libappindicator],
        [libappindicator],
        [http://launchpad.net/libappindicator])
AC_COPYRIGHT([Copyright 2009, 2010 Canonical])

AC_PREREQ([2.64])

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([1.11 -Wall tar-pax])

AM_MAINTAINER_MODE([enable])

AM_SILENT_RULES([yes])

# Check for programs
AC_PROG_CC
AM_PROG_CC_C_O

# Initialize libtool
LT_PREREQ([2.2])
LT_INIT

AC_PATH_PROG([GLIB_MKENUMS], [glib-mkenums])
AC_PATH_PROG([GLIB_GENMARSHAL], [glib-genmarshal])

PKG_PROG_PKG_CONFIG

###########################
# GTK Doc
###########################

GTK_DOC_CHECK([1.9])


###########################
# Dependencies
###########################

GTK_REQUIRED_VERSION=2.18
GTK3_REQUIRED_VERSION=2.91
GLIB_REQUIRED_VERSION=2.26
GIO_REQUIRED_VERSION=2.26
INDICATOR_REQUIRED_VERSION=0.4.93
DBUSMENUGTK_REQUIRED_VERSION=0.5.90
DBUS_GLIB_REQUIRED_VERSION=0.82

AC_ARG_WITH([gtk],
  [AS_HELP_STRING([--with-gtk],
    [Which version of gtk to use @<:@default=2@:>@])],
  [],
  [with_gtk=2])
AS_IF([test "x$with_gtk" = x3],
        [PKG_CHECK_MODULES(LIBRARY,  gtk+-3.0 >= $GTK3_REQUIRED_VERSION
                                     glib-2.0 >= $GLIB_REQUIRED_VERSION
                                     gio-2.0 >= $GIO_REQUIRED_VERSION
                                     indicator3-0.4 >= $INDICATOR_REQUIRED_VERSION
                                     dbusmenu-gtk3-0.4 >= $DBUSMENUGTK_REQUIRED_VERSION)
         AC_DEFINE(HAVE_GTK3, 1, [whether gtk3 is available])
        ],
      [test "x$with_gtk" = x2],
        [PKG_CHECK_MODULES(LIBRARY,  gtk+-2.0 >= $GTK_REQUIRED_VERSION
                                     glib-2.0 >= $GLIB_REQUIRED_VERSION
                                     gio-2.0 >= $GIO_REQUIRED_VERSION
                                     indicator-0.4 >= $INDICATOR_REQUIRED_VERSION
                                     dbusmenu-gtk-0.4 >= $DBUSMENUGTK_REQUIRED_VERSION)
        ],
      [AC_MSG_FAILURE([Value for --with-gtk was neither 2 nor 3])]
)
AM_CONDITIONAL(USE_GTK3, [test "x$with_gtk" = x3])

PKG_CHECK_MODULES(TESTDEPS, dbus-glib-1)

###########################
# GObject Introspection
###########################

GOBJECT_INTROSPECTION_CHECK([0.6.7])

PKG_CHECK_EXISTS([gobject-introspection-1.0 >= 0.10],
				 introspection_ten=yes,
				 introspection_ten=no)

AM_CONDITIONAL(INTROSPECTION_TEN, [test "x$introspection_ten" = "xyes"])

###########################
# Vala API Generation
###########################

AC_PATH_PROG([VALA_API_GEN], [vapigen])
AM_CONDITIONAL(HAVE_VAPIGEN, [test "x$VALA_API_GEN" != "x"])

###########################
# Vala Compiler support
###########################

AM_PROG_VALAC([0.14.0])
AM_CONDITIONAL(HAVE_VALAC, [test "x$VALAC" != "x"])

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

PKG_CHECK_MODULES(GAPI, gapi-2.0 >= $GLIB_SHARP_REQ_VERSION)
AC_PATH_PROG(GAPI_PARSER, gapi2-parser, no)
AC_SUBST(GAPI_PARSER)
AC_PATH_PROG(GAPI_CODEGEN, gapi2-codegen, no)
AC_SUBST(GAPI_CODEGEN)
AC_PATH_PROG(GAPI_FIXUP, gapi2-fixup, no)
AC_SUBST(GAPI_FIXUP)

# Check for nunit
AC_ARG_ENABLE([mono-test],
	AC_HELP_STRING([--disable-mono-test], [Disable mono test]),,
	[enable_mono_test=auto])

if test x"$enable_mono_test" != x"no" ; then
   PKG_CHECK_MODULES(NUNIT, nunit >= 2.4.7,
      [have_nunit=yes],
      [PKG_CHECK_MODULES(MONO_NUNIT, mono-nunit,
         [have_nunit=yes],
         [have_nunit=no])
   ])
   if test x${enable_mono_test} = xyes && test x${have_nunit} = xno; then
     AC_MSG_ERROR([Mono test configured but nunit not found])
   fi
else
   have_nunit=no
fi
AM_CONDITIONAL(BUILD_MONO_TEST, test x${have_nunit} = xyes)

###########################
# Check to see if we're local
###########################

with_localinstall="no"
AC_ARG_ENABLE(localinstall, AS_HELP_STRING([--enable-localinstall], [install all of the files localy instead of system directories (for distcheck)]), with_localinstall=$enableval, with_localinstall=no)

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

AC_MSG_CHECKING(for pygtk defs)
PYGTK_DEFSDIR=`$PKG_CONFIG --variable=defsdir pygtk-2.0`
AC_SUBST(PYGTK_DEFSDIR)
AC_MSG_RESULT($PYGTK_DEFSDIR)

AC_MSG_CHECKING(for pygtk codegen)
PYGTK_CODEGEN="$PYTHON `$PKG_CONFIG --variable=codegendir pygtk-2.0`/codegen.py"
AC_SUBST(PYGTK_CODEGEN)
AC_MSG_RESULT($PYGTK_CODEGEN)

#########################
# Check if build tests
#########################
AC_ARG_ENABLE([tests],
	AC_HELP_STRING([--disable-tests], [Disable tests]),,
	[enable_tests=yes])
AM_CONDITIONAL([WANT_TESTS], [test "x$enable_tests" != "xno"])

###########################
# Files
###########################

AC_CONFIG_FILES([
Makefile
src/Makefile
src/appindicator-0.1.pc
src/appindicator3-0.1.pc
bindings/Makefile
bindings/mono/Makefile
bindings/mono/appindicator-sharp.dll.config
bindings/mono/appindicator-sharp-0.1.pc
bindings/mono/app-indicator.sources.xml
bindings/mono/examples/Makefile
bindings/mono/examples/indicator-example
bindings/python/Makefile
bindings/python/appindicator.override
bindings/vala/Makefile
bindings/vala/examples/Makefile
tests/Makefile
example/Makefile
docs/Makefile
docs/reference/Makefile
docs/reference/version.xml
docs/reference/libappindicator-docs.sgml
])
AC_OUTPUT

###########################
# Results
###########################

AC_MSG_NOTICE([

Libappindicator Configuration:

	Prefix:        $prefix
	GTK+ Version:  $with_gtk
	Tests:         $enable_tests
	Mono tests:    $have_nunit
])
