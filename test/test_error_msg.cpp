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
#include <sstream>

#include <tinyparser.hpp>

using namespace std;
using namespace tipa;

TEST_CASE("test error position 1", "[error]")
{
    // the grammar 
    rule term = rule(tk_ident);
    rule op = rule("||") | rule("^") | rule("->");
    rule composition = term >> *(op >> term); 

    parser_context pc;

    SECTION("error in the first rule") {
        stringstream str("1234 ^ second");
        stringstream arr("^");
        pc.set_stream(str);
        bool f = parse_all(composition, pc);
        REQUIRE(not f);
        REQUIRE(pc.get_formatted_err_msg() ==
                "@[1:0]\n" 
                + str.str() + "\n"
                + arr.str() + "\n"
                "Error 1: Terminal rule failed\n");
    }

    SECTION("error missing second operand") {
        stringstream str("first ->"); 
        stringstream arr("-------^");
        pc.set_stream(str);
        bool f = parse_all(composition, pc);
        REQUIRE(not f);
        REQUIRE(pc.get_formatted_err_msg() ==
                "@[1:8]\n"
                + str.str() + "\n"
                + arr.str() + "\n"
                "Error 1: Terminal rule failed\n"
            );
    }

    // SECTION("error bad operator" ) {
    //     stringstream str("first * second");
    //     pc.set_stream(str);
    //     bool f = parse_all(composition, pc);
    //     REQUIRE(not f);
    //     REQUIRE(pc.get_formatted_err_msg() ==
    //             ""
    //         );
    // }
    
}
