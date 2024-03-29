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
#include <string>
#include <stack>
#include <iostream>
#include <sstream>
#include <memory>
#include <iomanip>

#include <tinyparser.hpp>
#include <property.hpp>

using namespace std;
using namespace tipa;

/*
  The kind of file I want to parse is the following:

  section_name1 {
     object_name1 {
         property1 : value1;
         property2 : value2;
         property3 : {
             subproperty1 : value;
             <etc>
         }
         <etc>
     }
     
  }

  section_name2 {
     <etc>
  }

  section_name3 {
     <etc>
  }

  In practice, each section is an hierarchical list of objects. So,
  the results for each section should be a tree. 

  Since the three sections are similar, in this example I write the
  parser for a single one. The idea is to generalise the parser so
  that I can chain three different versions of it.
*/


/*
  Visitor for printing the file
 */
class PrintVisitor : public Visitor<PropertyTypeList> {
    int level = 1;
public:
    void visit(PropertyNode &node) {
        cout << std::setw(level) << " ";         
        cout << "Visitor: Node : " << node.get_name() << endl;
        auto v = node.get_children();
        level += 4;
        for (auto x : v) {
            if (x == nullptr) 
                cout << "!!!! Problem !!!!" << endl;

            accept<PropertyTypeList>(x, *this);
        }
        level -= 4;
    }
    
    void visit(PropertyLeaf &node) {
        cout << std::setw(level) << " "; 
        cout << "Visitor:  Leaf  : name = " << node.get_name() << ", ";
        cout << "value = " << node.get_value() << "\n";
    }
};


int main()
{
    // forward declaration of the rules
    rule root_rule, root_name, plist, pnode, pleaf;

    // ----------- rules --------------
    // the top level rule is the section 
    root_rule = root_name >> rule('{') >> plist >> rule('}');
    // the name of the section 
    root_name = rule(tk_ident);
    // a list of property nodes
    plist = *pnode;
    // a node is a leaf node or another section 
    pnode = pleaf | root_rule;
    // a leaf node is an identifier, followed by a colon, followed by
    // another identifier and the semicolon.
    pleaf = rule(tk_ident) >> rule(':') >> rule(tk_ident) >> rule(';');

    // The tree builder
    PropertyBuilder b;

    // semantic of the rules
    // when you see a leaf, build a leaf node and add to the tree
    pleaf    .set_action([&b](parser_context &pc) { b.build_leaf(pc); });
    // when you see a section name, start building a section 
    root_name.set_action([&b](parser_context &pc) { b.build_root_begin(pc); });
    // when you finish a section, end building the section 
    root_rule.set_action([&b](parser_context &pc) { b.build_root_end(pc); });

    // the example file
    stringstream str("sys {id : peppe; cpu { name : core0; } ol : pluto; }");    

    // preparing the parser with the file
    parser_context pc; pc.set_stream(str);
                           
    try {
        root_rule.parse(pc);
    } catch(parse_exc &e) {
        cout << "Parse exception!" << endl;
        cout << pc.get_formatted_err_msg() << endl;
    }

    // now I visit the tree for printing it
    PrintVisitor pv;
    auto tree = b.get();
    accept<PropertyTypeList>(*tree.get(), pv);
}

