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

#define LEX_ADD   2
#define LEX_SUB   3
#define LEX_END   4

TEST_CASE( "Collecting two integers", "[collector]")
{
    stringstream str("37 + 52");
    parser_context pc;
    pc.set_stream(str);
    
    pair<int, int> result;
    auto f = [&result](parser_context &pc) {
        auto v = pc.collect_tokens();
        if (v.size() == 2) { 
            result.first = atoi(v[0].second.c_str());
            result.second = atoi(v[1].second.c_str());
        } else 
            cout << "Not enough elements" << endl;
    }; 
    
    rule op = rule('+') | rule('-');
    rule expr = rule(tk_int) >> op >> rule(tk_int) ;
    
    expr[f];

    REQUIRE(expr.parse(pc) == true);

    REQUIRE(result.first == 37);
    REQUIRE(result.second == 52);
}


TEST_CASE( "Collecting n integers", "[collector]")
{
    stringstream str("1 + 2 - 3 + 4 + 5 + ;");
    parser_context pc;
    pc.set_stream(str);
    
    vector<int> results;

    action_t f = [&results](parser_context &pc) {
        INFO("Calling the function to collect parameters");
        auto tv = pc.collect_tokens();
        for (unsigned int i = 0; i<tv.size(); i++) 
            // if (i % 2 == 0)
            results.push_back(atoi(tv[i].second.c_str()));
        //results.pop_back();
        return true;
    };

    vector<int> expect = {1, 2, 3, 4, 5};

    rule expr;
    rule op = rule('+') | rule('-');
    expr = rule(';') | (rule(tk_int) >> op >> expr);
    
    expr[f];

    REQUIRE(expr.parse(pc) == true);
    REQUIRE(results == expect);
}
