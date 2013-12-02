lib_LTLIBRARIES = libtinyparser.la

#bin_PROGRAMS = example 

AM_CPPFLAGS = $(BOOST_CPPFLAGS) -std=c++0x -Wall
AM_LDFLAGS  = $(BOOST_REGEX_LDFLAGS)
LIBS        = $(BOOST_REGEX_LIBS)

libtinyparser_la_SOURCES= lexer.cpp parser.cpp