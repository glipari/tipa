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
#include <vector>
#include <map>
#include <sstream>

#include <tinyparser.hpp>

using namespace std;
using namespace tipa;

/* Parsing a list of integers. 

   Notice that the rule is recursive, so numbers are read in
   right-to-left order. Therefore, we store each number at the
   beginning of the vector.  If we want to go left-to-right, we have
   to write an action for the internal rule (see next test).
*/ 
TEST_CASE("test a list of rules", "[list]")
{
    rule l = list_rule(rule(tk_int));

    stringstream str("1, 2, 3, 4");

    vector<int> values;

    l.set_action([&values](parser_context &pc) {
            int x;
            read_all(pc, x);
            values.insert(values.begin(), x);
        });
    
    parser_context pc;
    pc.set_stream(str);
    
    REQUIRE_NOTHROW(l.parse(pc));
    REQUIRE(values.size() == 4);

    REQUIRE(values[0] == 1);
    REQUIRE(values[1] == 2);
    REQUIRE(values[2] == 3);
    REQUIRE(values[3] == 4);
}

/* 
   Parsing a list of pairs. 

   In this case, the action is on the internal rule, therefore the
   pairs are correctly processed from left to right and we push them
   back in the vector in the correct order. 
*/
TEST_CASE("parse a list of more complex rules", "[list]")
{   
    rule rpair = rule('(') >> rule(tk_ident) >> rule(',') >> rule(tk_int) >> rule(')'); 

    vector<std::pair<std::string, int>> values;

    rpair.set_action([&values](parser_context &pc) {
            std::pair<std::string, int> p;
            read_all(pc, p.first, p.second);
            values.push_back(p);
        });
                      
    rule l = list_rule(std::move(rpair));

    stringstream str("(a, 1) , (b, 2), (c, 3), (d, 4)");
    
    parser_context pc;
    pc.set_stream(str);
    
    REQUIRE_NOTHROW(l.parse(pc));
    REQUIRE(values.size() == 4);

    REQUIRE(values[0].first == "a");
    REQUIRE(values[0].second == 1);
    REQUIRE(values[1].first == "b");
    REQUIRE(values[1].second == 2);
    REQUIRE(values[2].first == "c");
    REQUIRE(values[2].second == 3);
    REQUIRE(values[3].first == "d");
    REQUIRE(values[3].second == 4);
}

TEST_CASE("parsing a list of properties", "[list]")
{
    map<string, int> values;

    rule prop_name = keyword("wcet") | keyword("dline") | keyword("period") | keyword("offset");
    rule property = std::move(prop_name) >> rule('=') >> rule(tk_int);

    property.set_action([&values](parser_context &pc) {
            string name; int v;
            read_all(pc, name, v);
            values[name] = v;
        });
    
    rule root = rule("{") >> list_rule(std::move(property), ";") >> rule("}");

    stringstream str("{ wcet = 5; dline = 10; period = 15 }  ");

    parser_context pc;
    pc.set_stream(str);

    REQUIRE_NOTHROW(root.parse(pc));

    REQUIRE(values["wcet"] == 5);
    REQUIRE(values["dline"] == 10);
    REQUIRE(values["period"] == 15);
}
