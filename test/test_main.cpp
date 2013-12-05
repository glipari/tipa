#define CATCH_CONFIG_MAIN
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

static const int LEX_OP_PAR      = (LEX_LIB_BASE + 3);
static const int LEX_CL_PAR      = (LEX_LIB_BASE + 4);
static const int LEX_OP_SQUARE   = (LEX_LIB_BASE + 5);
static const int LEX_CL_SQUARE   = (LEX_LIB_BASE + 6);
static const int LEX_OP_BRACK    = (LEX_LIB_BASE + 7);
static const int LEX_CL_BRACK    = (LEX_LIB_BASE + 8);
static const int LEX_COMMA       = (LEX_LIB_BASE + 9);
static const int LEX_COLON       = (LEX_LIB_BASE + 10);
static const int LEX_SEMICOLON   = (LEX_LIB_BASE + 11);
static const int LEX_EQUALITY    = (LEX_LIB_BASE + 12);
static const int LEX_ASSIGNMENT  = (LEX_LIB_BASE + 13);

static const token tk_op_par(LEX_OP_PAR, "\\(");
static const token tk_cl_par(LEX_CL_PAR, "\\)");
static const token tk_op_sq(LEX_OP_SQUARE, "\\[");
static const token tk_cl_sq(LEX_CL_SQUARE, "\\]");
static const token tk_op_br(LEX_OP_BRACK, "\\{");
static const token tk_cl_br(LEX_CL_BRACK, "\\}");
static const token tk_comma(LEX_COMMA, ",");
static const token tk_colon(LEX_COLON, ":");
static const token tk_semicolon(LEX_SEMICOLON, ";");
static const token tk_equality(LEX_EQUALITY, "==");
static const token tk_assignment(LEX_ASSIGNMENT, ":=");

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
	    LEX_OP_PAR, LEX_INT, LEX_COMMA, LEX_INT, LEX_COMMA, LEX_INT, LEX_COMMA, 
	    LEX_INT, LEX_COMMA, LEX_IDENTIFIER, LEX_CL_PAR, LEX_COMMA,  LEX_INT, LEX_COMMA, 
	    LEX_IDENTIFIER, LEX_CL_PAR
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
	REQUIRE( r.first == LEX_INT );
	r = lex.get_token();
	REQUIRE( r.first == LEX_COMMA );
	r = lex.get_token();
	REQUIRE( r.first == LEX_INT );
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
	    {LEX_OP_PAR, "\\("},
	    {LEX_CL_PAR, "\\)"},
	    {LEX_IDENTIFIER, "^[^\\d\\W]\\w*"}
	});
    
    stringstream str("example ( ( word ) ( )\n" 
		     "***)");

    lex.set_stream(str);
    auto r = lex.get_token();
    REQUIRE( r.first == LEX_IDENTIFIER );
    REQUIRE( r.second == "example" );
    r = lex.get_token();
    REQUIRE( r.first == LEX_OP_PAR );
    REQUIRE( r.second == "(" );
    std::string s = lex.extract("(", ")");
    REQUIRE( s == " ( word ) ( )\n***");
}

TEST_CASE( "Extraction complex", "[lexer]" ) {
    ahead_lexer lex({
	    {LEX_OP_PAR, "\\/\\*"},
	    {LEX_CL_PAR, "\\*\\/"},
	    {LEX_IDENTIFIER, "^[^\\d\\W]\\w*"}
	});

    SECTION ( "no nesting" ) {
	stringstream str("pippo /* pluto */");
	
	lex.set_stream(str);
	auto r = lex.get_token();
	REQUIRE( r.first == LEX_IDENTIFIER );
	REQUIRE( r.second == "pippo" );
	r = lex.get_token();
	REQUIRE( r.first == LEX_OP_PAR );
	REQUIRE( r.second == "/*" );
	std::string s = lex.extract("/*", "*/");
	REQUIRE( s == " pluto ");
    }
    
    SECTION( "nesting" ) {
	stringstream str("/* /* pluto \n*/    */*/");
	
	lex.set_stream(str);
	auto r = lex.get_token();
	REQUIRE( r.first == LEX_OP_PAR );
	REQUIRE( r.second == "/*" );
	std::string s = lex.extract("/*", "*/");
	REQUIRE( s == " /* pluto \n*/    ");	
	r = lex.get_token();
	REQUIRE( r.first == LEX_CL_PAR );
	REQUIRE( r.second == "*/" );
	r = lex.get_token();
	REQUIRE( r.first == LEX_ERROR );
    }

    SECTION( "error" ) {
	stringstream str("/* /* pluto \n*/");
	lex.set_stream(str);
	auto r = lex.get_token();
	REQUIRE( r.first == LEX_OP_PAR );
	REQUIRE( r.second == "/*" );
	std::string s;
	REQUIRE_THROWS_AS(s = lex.extract("/*", "*/"), const char *);
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
	    LEX_OP_SQUARE, LEX_IDENTIFIER, LEX_ASSIGNMENT, 
	    LEX_IDENTIFIER, LEX_CL_SQUARE, LEX_SEMICOLON
	};
	lex.set_stream(str);
	check(lex, results);
    }
    SECTION ("second batch") {
	stringstream str("{c := d;};");

	std::vector<token_id> results = {
	    LEX_OP_BRACK, LEX_IDENTIFIER, LEX_ASSIGNMENT, LEX_IDENTIFIER, 
	    LEX_SEMICOLON, LEX_CL_BRACK, LEX_SEMICOLON
	};
	lex.set_stream(str);
	check(lex, results);
    }
    SECTION ("third batch") {
	stringstream str(":,:");

	std::vector<token_id> results = {
	    LEX_COLON, LEX_COMMA, LEX_COLON
	};
	lex.set_stream(str);
	check(lex, results);	
    }
    SECTION ("fourth batch") {
	stringstream str("e == 25;");

	std::vector<token_id> results = {
	    LEX_IDENTIFIER, LEX_EQUALITY, LEX_INT, 
	    LEX_SEMICOLON
	};
	lex.set_stream(str);
	check(lex, results);
    }

    SECTION ("fifth batch") {
	stringstream str("sys(mysys)");

	std::vector<token_id> results = {
	    LEX_IDENTIFIER, LEX_OP_PAR, LEX_IDENTIFIER, LEX_CL_PAR
	};
	lex.set_stream(str);
	check(lex, results);
    }

    SECTION ("brackets") {
	stringstream str("sys(mysys) {\n}");

	std::vector<token_id> results = {
	    LEX_IDENTIFIER, LEX_OP_PAR, LEX_IDENTIFIER, LEX_CL_PAR, 
	    LEX_OP_BRACK, LEX_CL_BRACK
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
	LEX_IDENTIFIER, LEX_IDENTIFIER, LEX_IDENTIFIER, LEX_IDENTIFIER 
    };

    lex.set_stream(str);
    check(lex, results);
}

TEST_CASE("from file in a stringstream", "[lexer]")
{
    ifstream file("lexer_multi.txt");
    ahead_lexer lex({tk_ident});
    std::vector<token_id> results = {
	LEX_IDENTIFIER, LEX_IDENTIFIER, LEX_IDENTIFIER, 
	LEX_IDENTIFIER, LEX_IDENTIFIER, LEX_IDENTIFIER 
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
	LEX_IDENTIFIER, LEX_IDENTIFIER, LEX_IDENTIFIER, 
	LEX_IDENTIFIER, LEX_IDENTIFIER, LEX_IDENTIFIER 
    };

    lex.set_stream(file);
    check(lex, results);
}

TEST_CASE("try token 1", "[lexer]")
{
    lexer lex;
    
    stringstream str("abc 123\n\t\n\tdef 456");

    lex.set_stream(str);

    REQUIRE(lex.try_token(tk_ident).first == LEX_IDENTIFIER);
    REQUIRE(lex.try_token(tk_int).first == LEX_INT);
    REQUIRE(lex.try_token(tk_ident).first == LEX_IDENTIFIER);
    REQUIRE(lex.try_token(tk_int).first == LEX_INT);
}

TEST_CASE("skip comments", "[lexer]")
{
    lexer lex;
    stringstream str("abc /*\ncomment*/ 123 // comment\n235");
    
    lex.set_stream(str);
    lex.set_comment("/*", "*/", "//");

    token_val tk = lex.try_token(tk_ident);
    REQUIRE(tk.first == LEX_IDENTIFIER);
    REQUIRE(tk.second == "abc");
    tk = lex.try_token(tk_int);
    REQUIRE(tk.first == LEX_INT);
    REQUIRE(tk.second == "123");
    tk = lex.try_token(tk_int);
    REQUIRE(tk.first == LEX_INT);
    REQUIRE(tk.second == "235");
}
