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


void myfunction(parser_context &pc)
{
    cout << "---- Parsed tokens: " << endl;
    auto v = pc.collect_tokens();
    for (auto x : v) {
	cout << x.second << ", "; 
    }
    cout << endl;
}


TEST_CASE("Null rule", "[parser]")
{
    stringstream str1("abc 12");
    stringstream str2("12");
    parser_context pc;

    SECTION("null rule itself") {
	pc.set_stream(str1);
	rule n = null();
	CHECK(n.parse(pc));
    }

    SECTION("0/1 rule") {
	rule opt = -rule("abc");
	rule num = rule(tk_int);
	rule expr = opt >> num;
	pc.set_stream(str1);
	CHECK(expr.parse(pc));

	pc.set_stream(str2);
	CHECK(expr.parse(pc));
    }

    SECTION("0/1 rule in the middle") {
	stringstream str1("{ 12 }");
	stringstream str2("{ }");
	
	rule final = rule('{') >> -rule(tk_int) >> rule('}');

	pc.set_stream(str1);
	CHECK(final.parse(pc));
	pc.set_stream(str2);
	CHECK(final.parse(pc));
    }

    SECTION("0/1 rule with composed rules") {
	stringstream str1("pippo { 12 } pluto");

	rule number = rule('{') > rule(tk_int) > rule('}');
	rule expr = keyword("pippo") >> -number >> keyword("pluto");
	number[myfunction];

	cout << "first" << endl;
	pc.set_stream(str1);
	CHECK(expr.parse(pc));
	cout << "------------------" << endl;
    }

    SECTION("0/1 rule with composed rules again") {
	stringstream str2("pippo pluto");
	stringstream str3("pippo { } pluto");

	rule number = rule('{') >> rule(tk_int) > rule('}');
	rule expr = keyword("pippo") >> -number >> keyword("pluto");
	number [myfunction];

	cout << "second" << endl;
	pc.set_stream(str2);
	CHECK(expr.parse(pc));
	cout << "------------------" << endl;

	cout << "Now the failing one " << endl;
	// this should fail!
	pc.set_stream(str3);
	try {
	    CHECK(!expr.parse(pc));
	} catch (std::exception &e) {
	    cout << "Exception: " << e.what() << endl;
	    throw;
	}
    }

    SECTION("0/1 failing") {
	stringstream str3("pippo { } pluto");

	rule number = rule('{') >> rule(tk_int) >> rule('}');
	rule expr = keyword("pippo") >> -number >> keyword("pluto");
	number[myfunction];

	pc.set_stream(str3);
	CHECK(!expr.parse(pc));
    }
}

TEST_CASE("Repetition rule followed by a line break", "[parser]")
{
    // cout << "Repetition rule follows by a line break" << endl;
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
