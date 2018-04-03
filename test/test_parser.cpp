/*
  Copyright 2015-2018 Giuseppe Lipari
  email: giuseppe.lipari@univ-lille.fr
  
  This file is part of TiPa.

  TiPa is free software: you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
  
  TiPa is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
  or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
  License for more details.
  
  You should have received a copy of the GNU General Public License
  along with TiPa. If not, see <http://www.gnu.org/licenses/>
 */
#include "catch.hpp"

#include <iostream>
#include <string>
#include <iterator>
#include <vector>
#include <sstream>

#include <tinyparser.hpp>

using namespace std;
using namespace tipa;


void print_tokens (parser_context &pc)
{
    auto v = pc.collect_tokens();
    cout << "N. collected tokens : " << v.size() << endl;
    for (auto x : v) cout << " - " << x.second << endl;
}


TEST_CASE( "Two terminals, separated by addition symbol", "[parser]")
{
    rule expr = rule(tk_int) >> rule('+') >> rule(tk_int);
    
    stringstream str("12 + 25");
    parser_context pc;
    pc.set_stream(str);

    CHECK(parse_all(expr,pc));
    // cout << "ERROR: " << pc.get_error_string() << endl;
    // print_tokens(pc);
    // cout << "eof : " << pc.eof() << endl;
}

TEST_CASE( "Addition / subtraction", "[parser]")
{
    stringstream str("37 + 52 - 64");
    parser_context pc;
    pc.set_stream(str);

    rule op = rule('+') | rule('-');
    
    rule expr = rule(tk_int) >> op >> rule(tk_int) >> op >> rule(tk_int);
    REQUIRE(parse_all(expr, pc) == true);
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

    REQUIRE(parse_all(expr, pc) == true);
}


TEST_CASE("Simpler sintax", "[parser]")
{
    lexer lex;

    SECTION("Without parenthesis") {
        stringstream str("abc 12");
        parser_context pc;
        pc.set_stream(str);
        rule expr = rule(tk_ident) >> rule(tk_int);
        REQUIRE(parse_all(expr, pc));
    }

    SECTION("With parenthesis") {
        stringstream str("abc(12);");
        parser_context pc;
        pc.set_stream(str);
        rule expr = rule(tk_ident) >> rule("(") 
                                   >> rule(tk_int) 
                                   >> rule(")") >> rule(";");
        REQUIRE(parse_all(expr, pc));
    }
    
}


void myfunction(parser_context &pc)
{
    // cout << "---- Parsed tokens: " << endl;
    // auto v = pc.collect_tokens();
    // for (auto x : v) {
    //     cout << x.second << ", "; 
    // }
    // cout << endl;
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

        CHECK(parse_all(expr, pc));

        pc.set_stream(str2);
        CHECK(parse_all(expr, pc));
    }

    SECTION("0/1 rule in the middle") {
        stringstream str1("{ 12 }");
        stringstream str2("{ }");
	
        rule final = rule('{') >> -rule(tk_int) >> rule('}');

        pc.set_stream(str1);
        CHECK(parse_all(final, pc));
        pc.set_stream(str2);
        CHECK(parse_all(final, pc));
    }

    SECTION("0/1 rule with composed rules") {
        stringstream str1("pippo { 12 } pluto");

        rule number = rule('{') > rule(tk_int) > rule('}');
        rule expr = keyword("pippo") >> -number >> keyword("pluto");
        number[myfunction];

        //cout << "first" << endl;
        pc.set_stream(str1);
        CHECK(parse_all(expr, pc));
        //cout << "------------------" << endl;
    }

    SECTION("0/1 rule with composed rules again") {
        stringstream str2("pippo pluto");
        stringstream str3("pippo { } pluto");

        rule number = rule('{') >> rule(tk_int) > rule('}');
        rule expr = keyword("pippo") >> -number >> keyword("pluto");
        number [myfunction];

        //cout << "second" << endl;
        pc.set_stream(str2);
        CHECK(parse_all(expr, pc));
        //cout << "------------------" << endl;

        //cout << "Now the failing one " << endl;
        // this should fail!
        pc.set_stream(str3);
        try {
            CHECK(!parse_all(expr, pc));
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
        CHECK(!parse_all(expr, pc));
    }
}

TEST_CASE("Repetition rule followed by a line break", "[parser]")
{
    stringstream str1("abc abc dd");
    stringstream str2("abc abc \n dd");
    parser_context pc1;
    parser_context pc2;
    SECTION("Section 1") {
        pc1.set_stream(str1);
        rule n1 = *(keyword("abc")) >> keyword("dd");
        CHECK(parse_all(n1, pc1));
    }
    SECTION("Section 2") {
        rule n2 = *(keyword("abc")) >> keyword("dd");	
        pc2.set_stream(str2);
        try {
            CHECK(parse_all(n2, pc2));
        } catch(parse_exc e) {
            cout << e.what() << endl;
            throw;
        }
    }
}

TEST_CASE("Greedy repetition", "[parser]")
{
    stringstream str1("123");
    stringstream str2("123 456\n 789");
    parser_context pc1;
    parser_context pc2;
    SECTION("Section 1") {
        pc1.set_stream(str1);
        rule n1 = *rule(tk_int);
        CHECK(parse_all(n1, pc1));
        CHECK(pc1.collect_tokens().size() == 1);
    }
    SECTION("Section 2") {
        rule n2 = *rule(tk_int);
        pc2.set_stream(str2);
        try {
            CHECK(parse_all(n2, pc2));
            auto v = pc2.collect_tokens();
            CHECK(v.size() == 3);
        } catch(parse_exc e) {
            cout << e.what() << endl;
            throw;
        }
    }
}


TEST_CASE("Greedy repetition of sequence", "[parser]")
{
    stringstream str1("{ abc 123 }");
    stringstream str2("{ abc 123 } { abc 456}\n\t\t{def 789}");
    parser_context pc1;
    parser_context pc2;
    SECTION("Section 1") {
        pc1.set_stream(str1);
        rule n1 = *(rule('{') >> rule(tk_ident) >> rule(tk_int) >> rule('}'));
        CHECK(parse_all(n1, pc1));
    }
    SECTION("Section 2") {
        rule n2 = *(rule('{') >> rule(tk_ident) >> rule(tk_int) >> rule('}'));
        pc2.set_stream(str2);
        try {
            CHECK(parse_all(n2, pc2));
            auto v = pc2.collect_tokens();
            CHECK(v.size() == 6);
        } catch(parse_exc e) {
            cout << e.what() << endl;
            throw;
        }
    }
}


TEST_CASE("Greedy repetition of alternatives", "[parser]")
{
    rule n1 = *(rule('{') >> rule(tk_ident) >> (rule(tk_int) | rule(tk_ident))  >> rule('}'));

    SECTION("Correct") {
        stringstream str("{ abc 123 } { abc def}\n\t\t{def 234}");
        parser_context pc;    
        pc.set_stream(str);
        
        //CHECK(n1.parse(pc));
        CHECK(parse_all(n1, pc));
        
        auto v = pc.collect_tokens();
        CHECK(v.size() == 6);
    }
    SECTION("Not Correct 1") {
        stringstream str("{ abc } {abc def}\n\t\t{def 234}");
        parser_context pc;    
        pc.set_stream(str);
        
        //CHECK(n1.parse(pc) == false);
        CHECK(parse_all(n1, pc) == false);
        
        auto v = pc.collect_tokens();
        CHECK(v.size() == 0);
    }
    SECTION("Not Correct 2") {
        stringstream str("{ abc 123 } {abc def}\n\t\t{def}");
        parser_context pc;    
        pc.set_stream(str);
        
        //CHECK(n1.parse(pc) == false);
        CHECK(parse_all(n1, pc) == false);
        
        auto v = pc.collect_tokens();
        CHECK(v.size() == 4);
    }
}


TEST_CASE("Using more than one rule", "[parser]")
{
    rule alt = rule(tk_ident) >> rule('=') >> (rule(tk_int) | rule(tk_ident));    
    rule root = *(rule('{') >>  alt  >> rule('}'));


    SECTION("Correct") {
        stringstream str("{ abc = 123 } { abc = def}\n\t\t{def = 234}");
        parser_context pc;
        
        pc.set_stream(str);
        CHECK(parse_all(root, pc));
    
        auto v = pc.collect_tokens();
        CHECK(v.size() == 6);
    }
    SECTION("Not Correct") {
        stringstream str("{ abc = 123 } { abc def}\n\t\t{def = 234}");
        parser_context pc;
        
        pc.set_stream(str);
        //CHECK(root.parse(pc) == false);
        CHECK(parse_all(root, pc) == false);
    
        auto v = pc.collect_tokens();
        CHECK(v.size() == 2);
    }
}




SCENARIO("Ownership test", "[parser]")
{
    GIVEN("A rule to build") {
        rule expr;
        parser_context c;
        stringstream str("{}");
        c.set_stream(str);

        WHEN("It contains another rule defined on the stack") {
            { 
                rule a = rule('{');
                expr = a >> rule('}'); 
            }
            THEN("Parsing raises an exception") {
                CHECK_THROWS_AS(parse_all(expr, c), parse_exc);
            }
        }
        WHEN("It contains a rule defined on the fly") {
            {
                expr = rule('{') >> rule('}');
            }
            THEN("No exception is thrown") {
                CHECK_NOTHROW(parse_all(expr, c));
            }
        }
        WHEN("The static rule passes the ownership") {
            { 
                rule a = rule('{');
                expr = std::move(a) >> rule('}'); 
            }
            THEN ("No exception is thrown") {
                CHECK_NOTHROW(parse_all(expr, c));
            }
        }
    }
}

