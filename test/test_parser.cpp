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


TEST_CASE("Null rule", "[parser]")
{
    stringstream str1("abc 12");
    stringstream str2("12");
    parser_context pc;
    SECTION("The null rule itself") {
	pc.set_stream(str1);
	rule n = null();
	CHECK(n.parse(pc));
    }
    SECTION("The 0/1 rule") {
	rule opt = - rule("abc");
	rule num = rule(tk_int);
	rule expr = opt >> num;
	pc.set_stream(str1);
	CHECK(expr.parse(pc));

	pc.set_stream(str2);
	CHECK(expr.parse(pc));
    }
}

TEST_CASE("Test the repetition rule followed by a line break: *xx >> yy", "[parser]")
{
    stringstream str1("abc abc dd");
    stringstream str2("abc abc \n dd");
    parser_context pc1;
    parser_context pc2;
    SECTION("Section 1") {
	pc1.set_stream(str1);
	rule n1 = *(keyword("abc")) >> keyword("dd");
	CHECK(n1.parse(pc1));
    }
    SECTION("Section 2") {
	rule n2 = *(keyword("abc")) >> keyword("dd");

	pc2.set_stream(str2);
	CHECK(n2.parse(pc2));
    }
}
