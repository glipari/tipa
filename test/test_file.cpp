#include "catch.hpp"

#include <iostream>
#include <string>
#include <iterator>
#include <vector>
#include <fstream>
#include <sstream>

#include <tipa/tinyparser.hpp>

using namespace std;
using namespace tipa;

#define LEX_EQ       3
#define LEX_QUOTES   4
#define LEX_TASK     5
#define LEX_SYS      6
#define LEX_PIPELINE 7


TEST_CASE( "parsing from file", "[parser]")
{
    ifstream file("struct.txt");
    if (!file.is_open()) FAIL("File struct.txt not found");
    
    rule prop_list;
    rule value = rule(tk_ident) | rule(tk_int); 
    rule type  = rule("task") | rule("sys") | rule("pipeline");
    rule name  = rule(tk_ident);
    rule prop  = rule(tk_ident) >> rule('=') >> value >> rule(';');
    rule prop_gen = prop_list | prop;

    prop_list = type >> rule('(') >> name >> rule(')') 
		     >> rule('{') >> *prop_gen >> rule('}') >> rule(';');
    
    SECTION("first from string") {
	stringstream str("sys(mysys) {\n task(t1) {		wcet=4; 		period=20;        };};");
	
	parser_context pc;
	pc.set_stream(str);
	try {
	    REQUIRE(prop_list.parse(pc));
	} catch(...) {
	    cout << pc.get_formatted_err_msg() << endl;
	}
    }

    SECTION("then from file") {
    	parser_context pc;
    	pc.set_stream(file);	
	try {
	    REQUIRE(prop_list.parse(pc));
	} catch(...) {
	    cout << pc.get_formatted_err_msg() << endl;
	}
    }
}
