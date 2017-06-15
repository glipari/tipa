#include "catch.hpp"

#include <iostream>
#include <string>

#include <tipa/wptr.hpp>

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
