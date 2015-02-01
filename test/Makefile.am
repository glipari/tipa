TESTS = test
check_PROGRAMS = test

AM_CPPFLAGS = $(BOOST_CPPFLAGS) -std=c++0x -Wall -I../src/ 
AM_LDFLAGS  = $(BOOST_REGEX_LDFLAGS) -L../src
LIBS        = $(BOOST_REGEX_LIBS) -ltinyparser-@TIPA_API_VERSION@

test_SOURCES = test_main.cpp test_parser.cpp \
	test_collector.cpp test_lexer.cpp  test_lexer2.cpp  \
	test_file.cpp test_wptr.cpp
