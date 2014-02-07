#include "catch.hpp"

#include <iostream>
#include <string>
#include <iterator>
#include <vector>
#include <sstream>

#include <tipa/tinyparser.hpp>

using namespace std;
using namespace tipa;

#define LEX_ADD   2
#define LEX_SUB   3

TEST_CASE( "Two terminals, separated by addition symbol", "[parser]")
{
    stringstream str("12 + 25");
    parser_context pc;
    pc.set_stream(str);

    rule expr = rule(tk_int) >> rule('+') >> rule(tk_int);

    REQUIRE(expr.parse(pc) == true);
}

TEST_CASE( "Addition / subtraction", "[parser]")
{
    stringstream str("37 + 52 - 64");
    parser_context pc;
    pc.set_stream(str);

    rule op = rule('+') | rule('-');
    
    rule expr = rule(tk_int) >> op >> rule(tk_int) >> op >> rule(tk_int);
    REQUIRE(expr.parse(pc) == true);
}


TEST_CASE("Recursive rule") 
{
    stringstream str("11 + 22 - 33 + 44 - PIPPO");
    parser_context pc;
    pc.set_stream(str);

    rule expr;
    rule op = rule('+') | rule('-');
    rule sum = rule(tk_int) >> op >> expr;
    expr = rule(tk_ident) | sum;

    REQUIRE(expr.parse(pc) == true);
}



TEST_CASE("Simpler sintax", "[parser]")
{
    lexer lex;

    SECTION("Without parenthesis") {
	stringstream str("abc 12");
	parser_context pc;
	pc.set_stream(str);
	rule expr = rule(tk_ident) >> rule(tk_int);
	REQUIRE(expr.parse(pc));
    }

    SECTION("With parenthesis") {
	stringstream str("abc(12);");
	parser_context pc;
	pc.set_stream(str);
	rule expr = rule(tk_ident) >> rule("(") 
				   >> rule(tk_int) 
				   >> rule(")") >> rule(";");
	REQUIRE(expr.parse(pc));
    }
    
}
