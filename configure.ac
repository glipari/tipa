dnl Process this file with autoconf to produce a configure script

AC_INIT(parse_lib, 0.1, g.lipari@sssup.it)

AM_INIT_AUTOMAKE([src], 1.1)
AM_CONFIG_HEADER(config.h)

AC_CONFIG_MACRO_DIR([m4])
AC_PROG_CXX
AC_LANG(C++)
AC_PROG_LIBTOOL

LT_INIT

BOOST_REQUIRE([1.34])
BOOST_REGEX

AC_CHECK_PROGS(RANLIB, ranlib)

AC_OUTPUT(Makefile src/Makefile
		   test/Makefile
		   example/Makefile,
		   [date > stamp-h])
