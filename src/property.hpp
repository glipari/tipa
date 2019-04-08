#ifndef __PROPERTY_HPP__
#define __PROPERTY_HPP__

#include <tuple>
#include <string>
#include <vector>
#include <memory>
#include <stack>

#include <genvisitor.hpp>
#include <tinyparser.hpp>

namespace tipa {

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
        std::vector< std::shared_ptr<AbsProperty> > list;
    public:
        PropertyNode(const std::string &n) : AbsProperty(n) {}
    
        void add_child(std::shared_ptr<AbsProperty> p) { list.push_back(p); }
        std::vector< std::shared_ptr<AbsProperty> > get_children() { return list; }
    };


    // Builds the property tree
    class PropertyBuilder {
        std::stack< std::shared_ptr<AbsProperty> > st;
        std::stack< int > level;
    public:
        void build_leaf(parser_context &pc);
        void build_root_begin(parser_context &pc);
        void build_root_end(parser_context &pc);
        std::shared_ptr<AbsProperty> get() { return st.top(); }
    };
}

#endif
