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
#include <genvisitor.hpp>

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
  the results for each section should be a tree.  The number and the
  names of the sections are fixed. I assume in this example they are
  =Platform=, =Tasks=, =Mapping= 

  Since the three sections are similar, in this example I write the
  parser for a single one. The idea is to generalise the parser so
  that I can chain three different versions of it.

  Since I am going to generate a tree, I write the typical classes for
  representing a tree, and a builder class to build the tree.
 */

class PropertyLeaf;
class PropertyNode;

using TypeList = std::tuple<PropertyLeaf, PropertyNode>;

// the abstract class for representing properties.
class AbsProperty {
    // every property has a name
    std::string name;  
public:
    AbsProperty(const std::string &n) : name(n) {}
    std::string get_name() { return name; }
    virtual ~AbsProperty() {}
};

// The leaf node of the tree: it contains a value
class PropertyLeaf : public AbsProperty, public Visitable<TypeList, PropertyLeaf> {
    std::string value;
public:
    PropertyLeaf(const std::string &n, const std::string &v) :
        AbsProperty(n), value(v) {}
    std::string get_value() { return value; }
};

// A list of abstract properties (intermediate node of the tree)
class PropertyNode : public AbsProperty, public Visitable<TypeList, PropertyNode> {
    // the list of children
    std::vector< shared_ptr<AbsProperty> > list;
public:
    PropertyNode(const std::string &n) : AbsProperty(n) {}
    
    void add_child(shared_ptr<AbsProperty> p) { list.push_back(p); }
    std::vector< shared_ptr<AbsProperty> > get_children() { return list; }
};


// Builds the property tree
class PropertyBuilder {
    stack< shared_ptr<AbsProperty> > st;
    stack< int > level;
public:
    void build_leaf(parser_context &pc) {
        //cout << "Builder: leaf has been found!" << endl;
        auto v = pc.collect_tokens(2);
        //for (auto t : v) cout << "Builder: Token : " << t.second << endl;
        if (v.size() < 2) throw string("Error in parsing leaf node");
        auto node = make_shared<PropertyLeaf>(PropertyLeaf(v[0].second, v[1].second));
        st.push(node);
    }

    void build_root_begin(parser_context &pc) {
        //cout << "Builder: Node has been found!" << endl;
        auto v = pc.collect_tokens(1);
        if (v.size() < 1) throw string("Error in parsing list node");
        // I know there is a node, so I need to store the current
        // stack level into a second stack
        level.push(st.size());
        //cout << "Saving current level " << st.size() << endl;
        // now I create a property node with the name that I have just seen
        auto node = make_shared<PropertyNode>(PropertyNode(v[0].second));
        st.push(node);
    }

    void build_root_end(parser_context &pc) {
        //cout << "Builder: Node completed!" << endl;
        int lev = level.top(); level.pop();
        //cout << "Builder: Level : " << lev << endl;
        //cout << "Builder: Current stack level : " << st.size() << endl;
        vector < shared_ptr<AbsProperty> > children;
        
        while (st.size() != (lev + 1)) {
            //cout << "Builder: Getting child" << endl;
            children.push_back(st.top()); st.pop();
        }
        //cout << "Builder: Getting node " << endl;
        shared_ptr<PropertyNode> pnode =
            std::static_pointer_cast<PropertyNode>(st.top());
        st.pop();
        
        for (auto x : children) 
            pnode->add_child(x);

        st.push(pnode);
        //cout << "Builder: Node pushed again" << endl;
    }
    
    shared_ptr<AbsProperty> get() {
        return st.top();
    }
};


/*
  Visitor for printing the file
 */
class PrintVisitor : public Visitor<TypeList> {
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

            accept<TypeList>(x, *this);
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
    // declare the rules
    rule root_rule, root_name, plist, pnode, pleaf;

    // rules
    root_rule = root_name >> rule('{') > plist > rule('}');
    root_name = rule(tk_ident);
    plist = *pnode;
    pnode = pleaf | root_rule;
    pleaf = rule(tk_ident) >> rule(':') > rule(tk_ident) > rule(';');

    // the tree builder
    PropertyBuilder b;
    using namespace std::placeholders;

    // semantic of the rules
    pleaf     [std::bind(&PropertyBuilder::build_leaf,       &b, _1)];
    root_name [std::bind(&PropertyBuilder::build_root_begin, &b, _1)];
    root_rule [std::bind(&PropertyBuilder::build_root_end,   &b, _1)];

    // the example file
    stringstream str("sys {id : peppe; cpu { name : core0; } ol : pluto; }");    

    // preparing the parser with the file
    parser_context pc; pc.set_stream(str);
    bool f = false;
    try {
        f = root_rule.parse(pc);
    } catch(parse_exc &e) {
        cout << "Parse exception!" << endl;
    }

    // now I visit the tree for printing it
    PrintVisitor pv;
    auto tree = b.get();
    accept<TypeList>(*tree.get(), pv);
}

