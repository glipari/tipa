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
#include <catch2/catch_test_macros.hpp>

#include <iostream>
#include <string>
#include <iterator>
#include <vector>
#include <fstream>
#include <sstream>

#include <tinyparser.hpp>

using namespace std;
using namespace tipa;

#define LEX_EQ       3
#define LEX_QUOTES   4
#define LEX_TASK     5
#define LEX_SYS      6
#define LEX_PIPELINE 7


TEST_CASE( "File parsing", "[parser]")
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

    //cout << prop_list.print() << endl;
    
    SECTION("File first from string") {
	stringstream str("sys(mysys) {\n task(t1) {		wcet=4; 		period=20;        };};");
	
	parser_context pc;
	pc.set_stream(str);
	try {
	    REQUIRE(prop_list.parse(pc));
	} catch(...) {
	    cout << pc.get_formatted_err_msg() << endl;
	}
    }

    SECTION("File then from file") {
    	parser_context pc;
    	pc.set_stream(file);	
	try {
	    REQUIRE(prop_list.parse(pc));
	} catch(...) {
	    cout << pc.get_formatted_err_msg() << endl;
	}
    }
}
