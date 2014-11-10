# Checks for existence of coverage tools:
#  * gcov
#  * lcov
#  * genhtml
#  * gcovr
# 
# Sets ac_cv_check_gcov to yes if tooling is present
# and reports the executables to the variables LCOV, GCOVR and GENHTML.
AC_DEFUN([AC_TDD_GCOV],
[
  AC_ARG_ENABLE(gcov,
  AS_HELP_STRING([--enable-gcov],
		 [enable coverage testing with gcov]),
  [use_gcov=$enableval], [use_gcov=no])

  if test "x$use_gcov" = "xyes"; then
  # we need gcc:
  if test "$GCC" != "yes"; then
    AC_MSG_ERROR([GCC is required for --enable-gcov])
  fi

  # Check if ccache is being used
  AC_CHECK_PROG(SHTOOL, shtool, shtool)
  case `$SHTOOL path $CC` in
    *ccache*[)] gcc_ccache=yes;;
    *[)] gcc_ccache=no;;
  esac

  if test "$gcc_ccache" = "yes" && (test -z "$CCACHE_DISABLE" || test "$CCACHE_DISABLE" != "1"); then
    AC_MSG_ERROR([ccache must be disabled when --enable-gcov option is used. You can disable ccache by setting environment variable CCACHE_DISABLE=1.])
  fi

  lcov_version_list="1.6 1.7 1.8 1.9 1.10 1.11"
  AC_CHECK_PROG(LCOV, lcov, lcov)
  AC_CHECK_PROG(GENHTML, genhtml, genhtml)

  if test "$LCOV"; then
    AC_CACHE_CHECK([for lcov version], glib_cv_lcov_version, [
      glib_cv_lcov_version=invalid
      lcov_version=`$LCOV -v 2>/dev/null | $SED -e 's/^.* //'`
      for lcov_check_version in $lcov_version_list; do
        if test "$lcov_version" = "$lcov_check_version"; then
          glib_cv_lcov_version="$lcov_check_version (ok)"
        fi
      done
    ])
  else
    lcov_msg="To enable code coverage reporting you must have one of the following lcov versions installed: $lcov_version_list"
    AC_MSG_ERROR([$lcov_msg])
  fi

  case $glib_cv_lcov_version in
    ""|invalid[)]
      lcov_msg="You must have one of the following versions of lcov: $lcov_version_list (found: $lcov_version)."
      AC_MSG_ERROR([$lcov_msg])
      LCOV="exit 0;"
      ;;
  esac

  if test -z "$GENHTML"; then
    AC_MSG_ERROR([Could not find genhtml from the lcov package])
  fi

  ac_cv_check_gcov=yes
  ac_cv_check_lcov=yes

  # Remove all optimization flags from CFLAGS
  changequote({,})
  CFLAGS=`echo "$CFLAGS" | $SED -e 's/-O[0-9]*//g'`
  changequote([,])

  # Add the special gcc flags
  COVERAGE_CFLAGS="-O0 -fprofile-arcs -ftest-coverage"
  COVERAGE_CXXFLAGS="-O0 -fprofile-arcs -ftest-coverage"	
  COVERAGE_LDFLAGS="-lgcov"

  # Check availability of gcovr
  AC_CHECK_PROG(GCOVR, gcovr, gcovr)
  if test -z "$GCOVR"; then
    ac_cv_check_gcovr=no
  else
    ac_cv_check_gcovr=yes
  fi

fi
]) # AC_TDD_GCOV
