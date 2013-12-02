#include <string>
#include <stack>
#include <iostream>
#include <fstream>
#include <memory>

#include <parser.hpp>
#include <property.hpp>

using namespace std;


int main(int argc, char *argv[])
{
    if (argc < 2) {
	cout << "Usage : " << argv[0] << " file_name" << endl;
	exit(-1);
    }
    
    rule prop_list;
    rule rstr = extract_rule("\""); 
    rule value = rstr | rule(tk_int); 
    rule type = keyword("task") | keyword("sys") | keyword("pipeline");
    rule name = rule(tk_ident);
    rule prop = rule(tk_ident) >> rule('=') >> (value | rstr) >> rule(';');
    rule comment = extract_line_rule("#");

    rule prop_gen = (prop_list | prop) >> *comment;

    prop_list = type >> rule('(') >> name >> rule(')') 
		     >> rule('{') >> (*prop_gen) >> rule('}')
		     >> rule(';'); 

    ifstream file(argv[1]);

    parser_context pc;
    pc.set_stream(file);
    
    bool f = false;
    try {
	f = prop_list.parse(pc);
	cout << "Parsing: " << f << endl;
    } catch(...) {}
    
    if (!f) cout << pc.get_formatted_err_msg() << endl;
}
