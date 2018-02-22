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

#include <wptr.hpp>

using namespace std;
using namespace tipa;

int wptr_counter = 0;

class MyClass {
public:
    MyClass() { wptr_counter++; }
    ~MyClass() { wptr_counter--; }
};

TEST_CASE("Weak pointer to int", "[wptr]")
{
    shared_ptr<int> sp(new int(5));
    WPtr<int> p1(sp);
    REQUIRE(*(p1.get()) == 5);
}

TEST_CASE("Weak pointer to MyClass, counting objects", "[wptr]")
{
    auto sp1 = make_shared<MyClass>();

    SECTION("Strong pointer") {
        REQUIRE(wptr_counter == 1);
	
        auto wptr1 = WPtr<MyClass>(sp1, WPTR_STRONG);
        sp1.reset();
        REQUIRE(wptr_counter == 1);
    }

    SECTION("Weak pointer") {
        REQUIRE(wptr_counter == 1);	
        auto wptr1 = WPtr<MyClass>(sp1, WPTR_WEAK);
        sp1.reset();
        REQUIRE(wptr_counter == 0);
        CHECK(!wptr1.get());
    }
}
