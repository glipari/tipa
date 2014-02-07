lib_LTLIBRARIES = libtinyparser-@TIPA_API_VERSION@.la
libtinyparser_@TIPA_API_VERSION@_la_SOURCES = lexer.cpp tinyparser.cpp

libtinyparser_@TIPA_API_VERSION@_la_CPPFLAGS = $(BOOST_CPPFLAGS) -std=c++0x -Wall
libtinyparser_@TIPA_API_VERSION@_la_LDFLAGS  = $(BOOST_REGEX_LDFLAGS) -version-info $(TIPA_SO_VERSION)
LIBS        = $(BOOST_REGEX_LIBS)

tipa_includedir = $(includedir)/tipa-$(TIPA_API_VERSION)
nobase_tipa_include_HEADERS = tipa/lexer.hpp tipa/tinyparser.hpp
