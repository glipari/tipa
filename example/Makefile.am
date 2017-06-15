noinst_PROGRAMS = arithmetic #tinyjson

AM_CPPFLAGS = $(BOOST_CPPFLAGS) -std=c++0x -Wall -I../src/ 
AM_LDFLAGS  = $(BOOST_REGEX_LDFLAGS) -L../src
LIBS        = $(BOOST_REGEX_LIBS) -ltinyparser-@TIPA_API_VERSION@

arithmetic_SOURCES = arithmetic.cpp 

#tinyjson_SOURCES = tinyjson.cpp


