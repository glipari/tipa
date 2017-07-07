#include <string>
#include <stack>
#include <iostream>
#include <sstream>
#include <memory>

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


class AbsProperty {
    std::string name;    
public:
    AbsProperty(const std::string &n) : name(n) {}
    std::string get_name() { return name; }
    virtual ~AbsProperty() {}
};


class PropertyLeaf : public AbsProperty, public Visitable<TypeList, PropertyLeaf> {
    std::string value;
public:
    PropertyLeaf(const std::string &n, const std::string &v) :
        AbsProperty(n), value(v) {}
    std::string get_value() { return value; }
};


class PropertyNode : public AbsProperty, public Visitable<TypeList, PropertyNode> {
    std::vector< shared_ptr<AbsProperty> > list;
public:
    PropertyNode(const std::string &n) : AbsProperty(n) {}
    
    void add_child(shared_ptr<AbsProperty> p) { list.push_back(p); }
    std::vector< shared_ptr<AbsProperty> > get_children() { return list; }
};


class PropertyBuilder {
    stack< shared_ptr<AbsProperty> > st;
    stack< int > level;
public:
    void build_leaf(parser_context &pc) {
        cout << "Leaf has been found!" << endl;
        auto v = pc.collect_tokens(2);
        for (auto t : v) cout << "Token : " << t.second << endl;
        if (v.size() < 2) throw string("Error in parsing leaf node");
        auto node = make_shared<PropertyLeaf>(PropertyLeaf(v[0].second, v[1].second));
        st.push(node);
    }

    void build_root_begin(parser_context &pc) {
        cout << "Node has been found!" << endl;
        auto v = pc.collect_tokens(1);
        if (v.size() < 1) cout << "Error in parsing list node" << endl;
        // I know there is a node, so I need to store the current
        // stack level into a second stack
        level.push(st.size());
        cout << "Saving current level " << st.size() << endl;
        // now I create a property node with the name that I have just seen
        auto node = make_shared<PropertyNode>(PropertyNode(v[0].second));
        st.push(node);
    }

    void build_root_end(parser_context &pc) {
        cout << "Node completed!" << endl;
        int lev = level.top(); level.pop();
        cout << "Level : " << lev << endl;
        cout << "Current stack level : " << st.size() << endl;
        vector < shared_ptr<AbsProperty> > children;
        
        while (st.size() != (lev + 1)) {
            cout << "Getting child" << endl;
            children.push_back(st.top()); st.pop();
        }
        cout << "Getting node " << endl;
        shared_ptr<PropertyNode> pnode =
            std::static_pointer_cast<PropertyNode>(st.top());
        st.pop();
        
        for (auto x : children) 
            pnode->add_child(x);

        st.push(pnode);
        cout << "Node pushed again" << endl;
    }
    
    shared_ptr<AbsProperty> get() {
        return st.top();
    }
};


class PrintVisitor : public Visitor<TypeList> {
public:
    void visit(PropertyNode &node) {
        cout << "Node : " << node.get_name() << "\n-------\n";
        cout << "Children : \n";
        auto v = node.get_children();
        for (auto x : v) {
            if (x == nullptr) 
                cout << "Problem !! " << endl;
            
            //x->accept(*this);
            auto p = dynamic_pointer_cast< AbstractVisitable<TypeList> >(x);
            if (p != nullptr)
                p->accept(*this);
            else cout << "Null dynamic cast" << endl;
        }
        
        cout << "-------\n";
    }
    
    void visit(PropertyLeaf &node) {
        cout << "  Leaf  : name = " << node.get_name() << ", ";
        cout << "value = " << node.get_value() << "\n";
    }
};




int main()
{
    rule root_rule, root_name, plist, pnode, pleaf;

    root_rule = root_name >> rule('{') > plist > rule('}');
    root_name = rule(tk_ident);
    plist = *pnode;
    pnode = pleaf | root_rule;
    pleaf = rule(tk_ident) >> rule(':') > rule(tk_ident) > rule(';');

    PropertyBuilder b;
    using namespace std::placeholders;
       
    pleaf     [std::bind(&PropertyBuilder::build_leaf,       &b, _1)];
    root_name [std::bind(&PropertyBuilder::build_root_begin, &b, _1)];
    root_rule [std::bind(&PropertyBuilder::build_root_end,   &b, _1)];

    stringstream str("sys {id : peppe; cpu { name : core0; } ol : pluto; }");    

    parser_context pc; pc.set_stream(str);
    bool f = false;
    try {
        f = root_rule.parse(pc);
    } catch(parse_exc &e) {
        cout << "Parse exception!" << endl;
    }
    
    PrintVisitor pv;

    auto *obj = dynamic_cast<AbstractVisitable<TypeList> *>(b.get().get());
    
    obj->accept(pv);
}

