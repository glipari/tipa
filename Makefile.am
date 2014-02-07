top="$(CURDIR)"/

ACLOCAL_AMFLAGS = ${ACLOCAL_FLAGS} -I m4

SUBDIRS= src/ test/ example/

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = tipa-$(TIPA_API_VERSION).pc

tipa_libincludedir = $(libdir)/tipa-$(TIPA_API_VERSION)/include
nodist_tipa_libinclude_HEADERS = tipaconfig.h
