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
#include <fstream>
#include <string>
#include <iterator>
#include <vector>
#include <sstream>

#include <lexer.hpp>

using namespace std;
using namespace tipa;

#define LEX_TYPE       1

static void check(ahead_lexer &lex, vector<token_id> &results) 
{
    for (unsigned int i=0; i<results.size(); i++) {
        auto r = lex.get_token();
        INFO(i);
        REQUIRE( r.first == results[i] );
    } 
    auto r = lex.get_token();
    REQUIRE(r.first == LEX_ERROR);
}

TEST_CASE( "first example", "[lexer]" ) {
    ahead_lexer lex({tk_int, tk_ident, tk_comma,
                tk_op_par, tk_cl_par, tk_op_sq, tk_cl_sq, 
                tk_op_br, tk_cl_br });    

    SECTION( "Correct" ) {
        std::string s = "(1, 2, 35, \n"
            "      \t   12, p32), 35,    \n"
            "\t\tpippo)\n";
	
        std::vector<token_id> results = {
            tk_op_par.get_name(), tk_int.get_name(), tk_comma.get_name(), tk_int.get_name(), tk_comma.get_name(), tk_int.get_name(), tk_comma.get_name(), 
            tk_int.get_name(), tk_comma.get_name(), tk_ident.get_name(), tk_cl_par.get_name(), tk_comma.get_name(),  tk_int.get_name(), tk_comma.get_name(), 
            tk_ident.get_name(), tk_cl_par.get_name()
        };
	
        stringstream str(s);
        lex.set_stream(str);
	
        check(lex, results);
    }

    SECTION( "Error Reporting" ) {
        std::string s = "32, 48 \n" 
            "     ## \\#23 )";
	
        stringstream str(s);
        lex.set_stream(str);
	
        auto r = lex.get_token();
        REQUIRE( r.first == tk_int.get_name() );
        r = lex.get_token();
        REQUIRE( r.first == tk_comma.get_name() );
        r = lex.get_token();
        REQUIRE( r.first == tk_int.get_name() );
        r = lex.get_token();
        REQUIRE( r.first == LEX_ERROR );
        auto pos = lex.get_pos();
        REQUIRE( pos.first == 2 );
        REQUIRE( pos.second == 5 );
        REQUIRE( lex.get_currline() == "     ## \\#23 )" );
    }
}


TEST_CASE( "Initialization", "[lexer]" ) {
    ahead_lexer lex({
            {1, "ABC"},
            {2, "DEF"},
            {3, "123"}
        });
    
    stringstream str("ABCDEF123");
    lex.set_stream(str);
    auto r = lex.get_token();
    REQUIRE( r.first == 1 );
    r = lex.get_token();
    REQUIRE( r.first == 2 );
    r = lex.get_token();
    REQUIRE( r.first == 3 );
}


TEST_CASE( "Extraction simple", "[lexer]" ) {
    ahead_lexer lex({
            {tk_op_par.get_name(), "\\("},
            {tk_cl_par.get_name(), "\\)"},
            {tk_ident.get_name(), "^[^\\d\\W]\\w*"}
        });
    
    stringstream str("example ( ( word ) ( )\n" 
                     "***)");

    lex.set_stream(str);
    auto r = lex.get_token();
    REQUIRE( r.first == tk_ident.get_name() );
    REQUIRE( r.second == "example" );
    r = lex.get_token();
    REQUIRE( r.first == tk_op_par.get_name() );
    REQUIRE( r.second == "(" );
    std::string s = lex.extract("(", ")");
    REQUIRE( s == " ( word ) ( )\n***");
}

TEST_CASE( "Extraction complex", "[lexer]" ) {
    ahead_lexer lex({
            {tk_op_par.get_name(), "\\/\\*"},
            {tk_cl_par.get_name(), "\\*\\/"},
            {tk_ident.get_name(), "^[^\\d\\W]\\w*"}
        });

    SECTION ( "no nesting" ) {
        stringstream str("pippo /* pluto */");
	
        lex.set_stream(str);
        auto r = lex.get_token();
        REQUIRE( r.first == tk_ident.get_name() );
        REQUIRE( r.second == "pippo" );
        r = lex.get_token();
        REQUIRE( r.first == tk_op_par.get_name() );
        REQUIRE( r.second == "/*" );
        std::string s = lex.extract("/*", "*/");
        REQUIRE( s == " pluto ");
    }
    
    SECTION( "nesting" ) {
        stringstream str("/* /* pluto \n*/    */*/");
	
        lex.set_stream(str);
        auto r = lex.get_token();
        REQUIRE( r.first == tk_op_par.get_name() );
        REQUIRE( r.second == "/*" );
        std::string s = lex.extract("/*", "*/");
        REQUIRE( s == " /* pluto \n*/    ");	
        r = lex.get_token();
        REQUIRE( r.first == tk_cl_par.get_name() );
        REQUIRE( r.second == "*/" );
        r = lex.get_token();
        REQUIRE( r.first == LEX_ERROR );
    }

    SECTION( "error" ) {
        stringstream str("/* /* pluto \n*/");
        lex.set_stream(str);
        auto r = lex.get_token();
        REQUIRE( r.first == tk_op_par.get_name() );
        REQUIRE( r.second == "/*" );
        std::string s;
        REQUIRE_THROWS_AS(s = lex.extract("/*", "*/"), parse_exc& );
    }
}

TEST_CASE("pre-defined symbols", "[lexer]")
{
    ahead_lexer lex({tk_op_sq, tk_op_par, tk_cl_par, tk_cl_sq, tk_op_br, tk_cl_br, 
                tk_comma, tk_semicolon, tk_equality, 
                tk_assignment, tk_int, tk_ident, tk_colon});

    SECTION ("first batch") {

        stringstream str("[a := b];");
	
        std::vector<token_id> results = {
            tk_op_sq.get_name(), tk_ident.get_name(), tk_assignment.get_name(), 
            tk_ident.get_name(), tk_cl_sq.get_name(), tk_semicolon.get_name()
        };
        lex.set_stream(str);
        check(lex, results);
    }
    SECTION ("second batch") {
        stringstream str("{c := d;};");

        std::vector<token_id> results = {
            tk_op_br.get_name(), tk_ident.get_name(), tk_assignment.get_name(), tk_ident.get_name(), 
            tk_semicolon.get_name(), tk_cl_br.get_name(), tk_semicolon.get_name()
        };
        lex.set_stream(str);
        check(lex, results);
    }
    SECTION ("third batch") {
        stringstream str(":,:");

        std::vector<token_id> results = {
            tk_colon.get_name(), tk_comma.get_name(), tk_colon.get_name()
        };
        lex.set_stream(str);
        check(lex, results);	
    }
    SECTION ("fourth batch") {
        stringstream str("e == 25;");

        std::vector<token_id> results = {
            tk_ident.get_name(), tk_equality.get_name(), tk_int.get_name(), 
            tk_semicolon.get_name()
        };
        lex.set_stream(str);
        check(lex, results);
    }

    SECTION ("fifth batch") {
        stringstream str("sys(mysys)");

        std::vector<token_id> results = {
            tk_ident.get_name(), tk_op_par.get_name(), tk_ident.get_name(), tk_cl_par.get_name()
        };
        lex.set_stream(str);
        check(lex, results);
    }

    SECTION ("brackets") {
        stringstream str("sys(mysys) {\n}");

        std::vector<token_id> results = {
            tk_ident.get_name(), tk_op_par.get_name(), tk_ident.get_name(), tk_cl_par.get_name(), 
            tk_op_br.get_name(), tk_cl_br.get_name()
        };
        lex.set_stream(str);
        check(lex, results);
    }
}

TEST_CASE("multi-line", "[lexer]")
{
    ahead_lexer lex({tk_ident});
    stringstream str("abc abc\n\t\n\tabc abc");
    std::vector<token_id> results = {
        tk_ident.get_name(), tk_ident.get_name(), tk_ident.get_name(), tk_ident.get_name() 
    };

    lex.set_stream(str);
    check(lex, results);
}

TEST_CASE("from file in a stringstream", "[lexer]")
{
    ifstream file("lexer_multi.txt");
    ahead_lexer lex({tk_ident});
    std::vector<token_id> results = {
        tk_ident.get_name(), tk_ident.get_name(), tk_ident.get_name(), 
        tk_ident.get_name(), tk_ident.get_name(), tk_ident.get_name() 
    };

    if (!file.is_open()) FAIL("File lexer_multi.txt not found");
    string tmp, total;
    while (file) {
        getline(file, tmp);
        total += tmp + "\n";
    }
    INFO(total);

    file.close();
    stringstream str(total);
    
    lex.set_stream(str);
    check(lex, results);
}


TEST_CASE("from file", "[lexer]")
{
    ifstream file("lexer_multi.txt");
    if (!file.is_open()) FAIL("File lexer_multi.txt not found");
    ahead_lexer lex({tk_ident});
    std::vector<token_id> results = {
        tk_ident.get_name(), tk_ident.get_name(), tk_ident.get_name(), 
        tk_ident.get_name(), tk_ident.get_name(), tk_ident.get_name() 
    };

    lex.set_stream(file);
    check(lex, results);
}

TEST_CASE("try token 1", "[lexer]")
{
    lexer lex;
    
    stringstream str("abc 123\n\t\n\tdef 456");

    lex.set_stream(str);

    REQUIRE(lex.try_token(tk_ident).first == tk_ident.get_name());
    REQUIRE(lex.try_token(tk_int).first == tk_int.get_name());
    REQUIRE(lex.try_token(tk_ident).first == tk_ident.get_name());
    REQUIRE(lex.try_token(tk_int).first == tk_int.get_name());
}

TEST_CASE("skip comments", "[lexer]")
{
    lexer lex;
    stringstream str("abc /*\ncomment*/ 123 // comment\n235");
    
    lex.set_stream(str);
    lex.set_comment("/*", "*/", "//");

    token_val tk = lex.try_token(tk_ident);
    REQUIRE(tk.first == tk_ident.get_name());
    REQUIRE(tk.second == "abc");
    tk = lex.try_token(tk_int);
    REQUIRE(tk.first == tk_int.get_name());
    REQUIRE(tk.second == "123");
    tk = lex.try_token(tk_int);
    REQUIRE(tk.first == tk_int.get_name());
    REQUIRE(tk.second == "235");
}
