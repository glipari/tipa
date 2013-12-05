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

#define LEX_EQ       3
#define LEX_QUOTES   4
#define LEX_TASK     5
#define LEX_SYS      6
#define LEX_PIPELINE 7

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
    for (int i=0; i<results.size(); i++) {
	auto r = lex.get_token();
	INFO(i);
	REQUIRE( r.first == results[i] );
    } 
    auto r = lex.get_token();
    REQUIRE(r.first == LEX_ERROR);
}

TEST_CASE("struct from file", "[lexer]")
{
    ifstream file("struct.txt");
    if (!file.is_open()) FAIL("File struct.txt not found");
    ahead_lexer lex({tk_int, 
	    {LEX_TASK, "task"}, 
	    {LEX_SYS, "sys"}, 
	    {LEX_PIPELINE, "pipeline"}, 
		tk_ident, 
		    tk_op_par, 
		    tk_cl_par, 
		    tk_semicolon, 
		    tk_colon, 
		    tk_op_br, 
		    tk_cl_br,
		    {LEX_EQ, "="},
		    {LEX_QUOTES, "\""}});


    std::vector<token_id> results = {
	LEX_SYS, LEX_OP_PAR, LEX_IDENTIFIER, LEX_CL_PAR, 
	LEX_OP_BRACK, LEX_TASK, LEX_OP_PAR, LEX_IDENTIFIER, LEX_CL_PAR, 
	LEX_OP_BRACK, LEX_IDENTIFIER, LEX_EQ, LEX_INT, LEX_SEMICOLON,
	LEX_IDENTIFIER, LEX_EQ, LEX_INT, LEX_SEMICOLON,
	LEX_CL_BRACK, LEX_SEMICOLON, 
	LEX_CL_BRACK, LEX_SEMICOLON
    };

    lex.set_stream(file);
    check(lex, results);
}

TEST_CASE("save and restore", "[lexer]")
{
    ahead_lexer lex({tk_int, tk_ident});

    SECTION("simple") {
	stringstream str("123 abc 789");
	lex.set_stream(str);
	token_val tk = lex.get_token();
	REQUIRE(tk.second == "123");
	lex.save();
	tk = lex.get_token();
	REQUIRE(tk.second == "abc");
	lex.restore();
	tk = lex.get_token();
	REQUIRE(tk.second == "abc");
	tk = lex.get_token();
	REQUIRE(tk.second == "789");    
    }
    SECTION("multi-line") {
	stringstream str("123\nabc\n789\ndef");
	lex.set_stream(str);
	token_val tk = lex.get_token();
	REQUIRE(tk.second == "123");
	INFO("Pos: " << lex.get_pos().first << ", " << lex.get_pos().second);
	lex.save();
	tk = lex.get_token();
	REQUIRE(tk.second == "abc");
	tk = lex.get_token();
	REQUIRE(tk.second == "789");    
	lex.restore();
	INFO("After restore, pos: " << lex.get_pos().first << ", " << lex.get_pos().second);
	tk = lex.get_token();
	REQUIRE(tk.second == "abc");
	tk = lex.get_token();
	REQUIRE(tk.second == "789");    
	tk = lex.get_token();
	REQUIRE(tk.second == "def");    
    }
}


