AC_PREREQ(2.61)
AC_INIT([tinyframe], [0.1.1], [todo@dns-oarc.net], [tinyframe], [https://github.com/DNS-OARC/tinyframe/issues])
AC_SUBST([TINYFRAME_VERSION_MAJOR], [0000])
AC_SUBST([TINYFRAME_VERSION_MINOR], [0001])
AC_SUBST([TINYFRAME_VERSION_PATCH], [0001])
AC_SUBST([TINYFRAME_LIBRARY_VERSION], [0:1:0])
AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects])
AC_CONFIG_SRCDIR([src/tinyframe.c])
AC_CONFIG_HEADER([src/config.h])
AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.
AC_PROG_CC
AM_PROG_CC_C_O
AC_CANONICAL_HOST
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])
LT_INIT

# Check --enable-warn-all
AC_ARG_ENABLE([warn-all], [AS_HELP_STRING([--enable-warn-all], [Enable all compiler warnings])], [AX_CFLAGS_WARN_ALL()])

# Check --with-extra-cflags
AC_ARG_WITH([extra-cflags], [AS_HELP_STRING([--with-extra-cflags=CFLAGS], [Add extra CFLAGS])], [
  AC_MSG_NOTICE([appending extra CFLAGS... $withval])
  AS_VAR_APPEND(CFLAGS, [" $withval"])
])

# Check --with-extra-ldflags
AC_ARG_WITH([extra-ldflags], [AS_HELP_STRING([--with-extra-ldflags=LDFLAGS], [Add extra LDFLAGS])], [
  AC_MSG_NOTICE([appending extra LDFLAGS... $withval])
  AS_VAR_APPEND(LDFLAGS, [" $withval"])
])

# Check --enable-trace
AH_TEMPLATE([TINYFRAME_TRACE], [Defined if trace of Frame Streams is enabled])
AC_ARG_ENABLE([trace], [AS_HELP_STRING([--enable-trace], [Enable trace output of Frame Streams])], [AC_DEFINE([TINYFRAME_TRACE])])

# Check --enable-gcov
AC_ARG_ENABLE([gcov], [AS_HELP_STRING([--enable-gcov], [Enable coverage testing])], [
  coverage_cflags="--coverage -g -O0 -fno-inline -fno-inline-small-functions -fno-default-inline"
  AC_MSG_NOTICE([enabling coverage testing... $coverage_cflags])
  AS_VAR_APPEND(CFLAGS, [" $coverage_cflags"])
])
AM_CONDITIONAL([ENABLE_GCOV], [test "x$enable_gcov" != "xno"])
AM_EXTRA_RECURSIVE_TARGETS([gcov])

# pkg-config
PKG_INSTALLDIR

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([endian.h sys/endian.h machine/endian.h])

# Checks for library functions.

# Output Makefiles
AC_CONFIG_FILES([
    Makefile
	libtinyframe.pc
    src/Makefile
    src/test/Makefile
    src/tinyframe/version.h
])
AC_OUTPUT
