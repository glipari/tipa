dnl Process this file with autoconf to produce a configure script

AC_INIT([Tipa], [0.1], [g.lipari@sssup.it], [tipa], [http://github.com/glipari/tipa])

AM_INIT_AUTOMAKE([1.10 -Wall no-define])
AM_CONFIG_HEADER([config.h tipaconfig.h])

AC_CONFIG_MACRO_DIR([m4])
AC_PROG_CXX
AC_LANG(C++)
AC_PROG_LIBTOOL
LT_INIT([disable-static])

#BOOST_REQUIRE([1.34])
#BOOST_REGEX

AC_CHECK_PROGS(RANLIB, ranlib)

AC_SUBST([TIPA_SO_VERSION], [1:0:0])
AC_SUBST([TIPA_API_VERSION], [1.0])

AC_CONFIG_FILES([Makefile tipa-${TIPA_API_VERSION}.pc:tipa.pc.in
		 src/Makefile	
		 test/Makefile
		 example/Makefile],
		 [date > stamp-h])

AC_OUTPUT
