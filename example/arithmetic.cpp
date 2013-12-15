#include <string>
#include <stack>
#include <iostream>
#include <sstream>
#include <memory>

#include <tinyparser.hpp>

using namespace std;
using namespace tipa;

#define LEX_PLUS    1
#define LEX_MINUS   2
#define LEX_MULT    3
#define LEX_DIV     4

class tree_node {
public:
    virtual int compute() = 0;
    virtual ~tree_node() {}
};

class op_node : public tree_node {
protected:
    shared_ptr<tree_node> left;
    shared_ptr<tree_node> right;
public:
    void set_left(shared_ptr<tree_node> l) {
	left = l;
    }
    void set_right(shared_ptr<tree_node> r) {
	right = r;
    }
};

class leaf_node : public tree_node {
    int value;
public:
    leaf_node(int v) : value(v) {}
    virtual int compute() { return value; }
};

#define OP_NODE_CLASS(xxx,sym)		 \
    class xxx##_node : public op_node {  \
    public:                              \
      virtual int compute() {            \
	int l = left->compute();         \
	int r = right->compute();        \
	return l sym r;                  \
      }                                  \
    }

OP_NODE_CLASS(plus,+);
OP_NODE_CLASS(minus,-);
OP_NODE_CLASS(mult,*);
OP_NODE_CLASS(div,/);

class builder {
    stack< shared_ptr<tree_node> > st;
public:
    void make_leaf(parser_context &pc) {
	auto x = pc.collect_tokens();
        if (x.size() < 1) throw string("Error in collecting integer");
	int v = atoi(x[x.size()-1].second.c_str());
	auto node = make_shared<leaf_node>(v);
	st.push(node);
    } 

    template<class T>
    void make_op(parser_context &pc) {
	auto r = st.top(); st.pop();
	auto l = st.top(); st.pop();
	auto n = make_shared<T>();
	n->set_left(l);
	n->set_right(r);
	st.push(n);
    }
    
    int get_size() { return st.size(); }

    shared_ptr<tree_node> get_tree() {
	return st.top();
    }
};

int main()
{
    // These are the parsing rules
    rule expr, primary, term, 
	op_plus, op_minus, op_mult, op_div, r_int;
    
    // An expression is sequence of terms separated by + or -
    expr = term >> *(op_plus | op_minus);
    op_plus = rule('+') > term;    
    op_minus = rule('-') > term;

    // A term is a sequence of primaries, separated by * or /
    term = primary >> *(op_mult | op_div);
    op_mult = rule('*') > primary;
    op_div = rule('/') > primary;

    // A primary is either an integer or an expression within parenthesis
    primary = r_int | 
	rule('(') >> expr >> rule(')');

    // An integer is an integer!
    r_int = rule(tk_int);
    
    // The following is used to build the syntax tree, 
    // which is used later for the calculations
    builder b; 
    using namespace std::placeholders;
    
    r_int   [std::bind(&builder::make_leaf,           &b, _1)];
    op_plus [std::bind(&builder::make_op<plus_node>,  &b, _1)];
    op_minus[std::bind(&builder::make_op<minus_node>, &b, _1)];
    op_mult [std::bind(&builder::make_op<mult_node>,  &b, _1)];
    op_div  [std::bind(&builder::make_op<div_node>,   &b, _1)];

    /*****************************************************/

    string input;
    cout << "Enter an arithmethic expression" << endl;
    cout << "Example: 3 - (2 + 1)" << endl;
    getline(cin, input);

    cout << "String to parse: " << input << endl;
    // the string to be parsed
    stringstream str(input);

    // given as input
    // preparing the "context" of the parser
    parser_context pc;
    pc.set_stream(str);

    // We now parse, and build the syntax tree at once
    bool f = false;
    try {
	f = expr.parse(pc);
    } catch(parse_exc &e) {
	cout << "Parse exception!" << endl;
    }

    if (!f) {
	cout << pc.get_formatted_err_msg();
    } else {
	// Take the tree and return the result
	auto tr = b.get_tree();
	cout << "Result = " << tr->compute() << endl; 
    }
}
