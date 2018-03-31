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
#include <utility>
#include <genvisitor.hpp>
#include <vector>

using namespace std;
using namespace tipa;

struct base {
    virtual ~base() {}
};

class der1;
class der2;
using TypeList = std::tuple<der1, der2>;


struct der1 : public base, public Visitable<TypeList, der1> {
    int a;
};

struct der2 : public base, public Visitable<TypeList, der2> {
    string name;
};


struct MyVisitor : public Visitor<TypeList> {
    int x1 = 0;
    int x2 = 0;
    void visit(der1 &d) {
        cout << "Der 1 , a = " << d.a << endl;
        x1++;
        
    }
    void visit(der2 &d) {
        cout << "Der 1 , name = " << d.name << endl;
        x2++;
    }
};
    

TEST_CASE("testing visitor", "[visitor]")
{
    der1 d1; d1.a = 5;
    der2 d2; d2.name = "Peppe";

    vector<AbstractVisitable<TypeList> *> v;
    v.push_back(&d1);
    v.push_back(&d2);

    MyVisitor vis;

    for (auto x : v) {
        x->accept(vis);
    }
    REQUIRE(vis.x1 == 1);
    REQUIRE(vis.x2 == 1);

    // Now I try to dynamic_cast to the base class
    for (auto x : v) {
        base *b = dynamic_cast<base *>(x);
        REQUIRE (b != nullptr);
    }
}

TEST_CASE("testing dynamic_cast", "[visitor]")
{
    der1 d1; d1.a = 5;
    der2 d2; d2.name = "Peppe";

    vector<base *> v; v.push_back(&d1); v.push_back(&d2);
    MyVisitor vis;

    for (auto x : v) {
        dynamic_cast<AbstractVisitable<TypeList>*>(x)->accept(vis);
    }
    REQUIRE(vis.x1 == 1);
    REQUIRE(vis.x2 == 1);
}

