#include <string>
#include <stack>
#include <iostream>
#include <sstream>
#include <fstream>
#include <memory>
#include <tuple>

#include <tinyparser.hpp>
#include <genvisitor.hpp>

using namespace std;
using namespace tipa;

/*
  Example of language for generating a makefile:

    global { cxxflags {-Wall} libflags {} } 

    exec { name {exec_name} srcs {file.cpp, file2.cpp, file3.cpp} }
    exec { name {exec_name} srcs {file4.cpp, file5.cpp, file2.cpp} }
    exec { name {exec_name} srcs {file.cpp}, lib {-lrt} }

  It generates the corresponding makefile.
*/

// storing the flags
string cxxflags, libflags;
using Executable = tuple<string, vector<string>, string>;
vector<Executable> all_execs;

rule create_grammar()
{
    token tk_srcfile = create_lib_token("\\w+([\\.]\\w*)?");

    // the language
    rule file_list = rule(tk_srcfile) >> *(rule(',') >> rule(tk_srcfile));
    rule cxxflags_rule = keyword("cxxflags") >> extract_rule("{", "}");
    cxxflags_rule.set_action(([](parser_context &pc) {
                cxxflags = pc.get_last_token().second;
                //cout << "CXXFLAGS = " << cxxflags << endl;
                pc.collect_tokens();
            }));


    rule libflags_rule = keyword("libflags") >> extract_rule("{", "}");
    libflags_rule.set_action(([](parser_context &pc) {
                libflags = pc.get_last_token().second;
                //cout << "LIBFLAGS = " << libflags << endl;
                pc.collect_tokens();
            }));
    
    rule global_rule = keyword("global") >> rule('{') >> -std::move(cxxflags_rule) >> -std::move(libflags_rule) >> rule('}');
    
    rule name_rule = keyword("name") >> rule('{') >> rule(tk_ident) >> rule('}');
    rule srcs_rule = keyword("srcs") >> rule('{') >> std::move(file_list) >> rule('}');
    rule lib_rule = keyword("lib") >> extract_rule("{", "}");
    rule exec_rule = keyword("exec") >> rule('{') >> std::move(name_rule) >> std::move(srcs_rule) >> -std::move(lib_rule) >> rule('}');
        
    exec_rule.set_action( ([] (parser_context &pc){
                auto v = pc.collect_tokens();
                Executable exec;
                if (v.size() < 4) {
                    cout <<"ERROR IN PARSING, LESS THAN 4 ELEMENTS IN EXEC, SOMETHING IS WRONG" << endl;
                    exit(-1);
                }
                //cout << "Executable : " << endl;
                //for (auto x : v) cout << x.second << endl;
                unsigned i = 0;
                while (i < v.size()) { 
                    if (v[i].second == "name") get<0>(exec) = v[++i].second;
                    else if (v[i].second == "lib") get<2>(exec) = v[++i].second;
                    else if (v[i].second == "srcs") {
                        ++i;
                        while (i < v.size()) {
                            if (v[i].second != "lib") get<1>(exec).push_back(v[i++].second);
                            else break;
                        }
                    }
                    else i++;
                }
                // printing
                // cout << "Exec name: " << get<0>(exec) << endl;
                // cout << "srcs: ";
                // for (auto x : get<1>(exec)) cout << x << ", ";
                // cout << endl;
                // cout << "libs: " << get<2>(exec) << endl;
                all_execs.push_back(exec);
            }));

    rule root_rule = std::move(global_rule) >> *std::move(exec_rule);
    
    return root_rule;
}

void makefile_gen(const std::string &cxxflags, const std::string &libflags, const vector<Executable> &all_execs)
{    
    // now let's generate the makefile
    ofstream output("mymakefile");
    output << "CXXFLAGS = " << cxxflags << " -MMD" <<endl;
    output << "LDFLAGS = " << libflags << endl;
    output << "CPP=g++\nLD=g++\n\n.SUFFIXES:\n.SUFFIXES: .o .cpp\n" << endl;

    output << "EXECS = ";
    for (auto x : all_execs) {
        output << get<0>(x) << " ";
    }
    output << "\n" << endl;

    for (auto x : all_execs) {
        output << "SRCS_" + get<0>(x) << " = ";
        for (auto y : get<1>(x)) output << y << " ";
        output << "\n";
        output << "OBJS_" + get<0>(x) << " = ";
        output << "${SRCS_" + get<0>(x) << ":.cpp=.o}\n\n";
    }

    output << "ALL_SRCS = ";
    for (auto x : all_execs) 
        output << "${SRCS_" + get<0>(x) << "} ";
    output << endl;

    output << "ALL_DEPS =  ${ALL_SRCS:.cpp=.d}\n" << endl;

    output << "all : $(EXECS)\n" << endl;        

    output << "-include $(ALL_DEPS)\n" << endl;        

    output << ".cpp.o:\n\t$(CPP) $(CXXFLAGS) -c $<\n\n";
    for (auto x : all_execs) {
        output << get<0>(x) << " : $(OBJS_" <<  get<0>(x) << ")\n";
        output << "\t$(LD) -o $@ $^ $(LDFLAGS) " << get<2>(x) << "\n\n";
    }
    output << "clean:\n";
    output << "\trm -rf *.o\n";
    output << "\trm -rf $(EXECS)\n\n";
    output << "cleanedit:\n";
	output << "\trm -rf *~\n";

    output.close();
}


int main(int argc, const char *argv[])
{
    rule root_rule = create_grammar();

    parser_context pc;
    
    ifstream fstr;
    stringstream str("global { cxxflags { -Wall -std=c++17 } } \n\n"
                     "exec { name {prog} srcs {prog.cpp, share.cpp} lib { -lrt } }\n" 
                     "exec { name {tool} srcs {tool.cpp, share.cpp} }");
    
    if (argc == 1) {
        pc.set_stream(str);
    }
    else {
        fstr.open(argv[1]);
        pc.set_stream(fstr);
    }
    
    bool f = false;
    try {
        f = parse_all(root_rule, pc);
        cout << "parsing is " << boolalpha << f << endl;
        if (!f) {
            cout << pc.get_formatted_err_msg() << endl;
        }
    } catch(parse_exc &e) {
        cout << "Parse exception!" << endl;
        cout << pc.get_formatted_err_msg() << endl;
    }

    if (!f) return 0;

    makefile_gen(cxxflags, libflags, all_execs);
}


