#include "catch.hpp"

#include <iostream>
#include <string>
#include <iterator>
#include <vector>
#include <sstream>

#include <parser.hpp>

using namespace std;

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
	if (v.size() == 3) { 
	    result.first = atoi(v[0].second.c_str());
	    result.second = atoi(v[2].second.c_str());
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

    auto f = [&results](parser_context &pc) {
	INFO("Calling the function to collect parameters");
	auto tv = pc.collect_tokens();
	for (unsigned int i = 0; i<tv.size(); i++) 
	    if (i % 2 == 0)
		results.push_back(atoi(tv[i].second.c_str()));
	//results.pop_back();
	return true;
    };

    vector<int> expect = {1, 2, 3, 4, 5, 0};

    rule expr;
    rule op = rule('+') | rule('-');
    expr = rule(';') | (rule(tk_int) >> op >> expr);
    
    expr[f];

    REQUIRE(expr.parse(pc) == true);
    REQUIRE(results == expect);
}
