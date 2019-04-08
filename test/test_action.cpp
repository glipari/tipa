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

TEST_CASE("test the read_vars() variadic function", "[action]")
{
    rule op = rule('+') | rule('-');
    rule expr = rule(tk_int) >> op >> rule(tk_int) >> op >> rule(tk_int);

    int n1 = 0, n2 = 0, n3 = 0;
    
    expr.read_vars(n1, n2, n3);

    stringstream str("37 + 52 + 12");
    parser_context pc;
    pc.set_stream(str);

    REQUIRE_NOTHROW(expr.parse(pc));
    
    REQUIRE(n1 == 37);
    REQUIRE(n2 == 52);
    REQUIRE(n3 == 12);
}


TEST_CASE("read_vars() on strings", "[action]")
{
    rule simple = rule(tk_ident) >> rule(tk_int) >> rule(tk_ident);
    string s1 = "", s2 = ""; int n;
    simple.read_vars(s1, n, s2);

    stringstream str("abc 5 def");

    parser_context pc;
    pc.set_stream(str);

    REQUIRE_NOTHROW(simple.parse(pc));

    REQUIRE(s1 == "abc");
    REQUIRE(s2 == "def");
    REQUIRE(n == 5);
}

TEST_CASE("read_vars() variable length rules", "[action]")
{
    rule simple = rule(tk_ident) >> *( rule(tk_int) >> rule(tk_ident) );
    string s1 = "", s2 = ""; int n;
    simple.read_vars(s1, n, s2);

    stringstream str("abc 5 def");

    parser_context pc;
    pc.set_stream(str);

    REQUIRE_NOTHROW(simple.parse(pc));

    REQUIRE(s1 == "abc");
    REQUIRE(s2 == "def");
    REQUIRE(n == 5);
}


TEST_CASE("read_vars(): more complex patterns", "[action]")
{
    rule command = rule(tk_ident) >> rule('(') >> rule(tk_ident) >> *( rule(',') >> rule(tk_int) ) >> rule(')');

    string cmd, par1;
    int p1, p2, p3;

    command.read_vars(cmd, par1, p1, p2, p3);
    
    stringstream str("system(cpu, 145, 12, 23)");
    parser_context pc;
    pc.set_stream(str);

    REQUIRE_NOTHROW(command.parse(pc));
    
    REQUIRE(cmd == "system");
    REQUIRE(par1 == "cpu");
    REQUIRE(p1 == 145);
    REQUIRE(p2 == 12);
    REQUIRE(p3 == 23);    
}
