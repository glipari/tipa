/*
  Copyright 2015-2023 Giuseppe Lipari
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
#ifndef __PROPERTY_HPP__
#define __PROPERTY_HPP__

#include <tuple>
#include <string>
#include <vector>
#include <memory>
#include <stack>

#include <genvisitor.hpp>
#include <tinyparser.hpp>

/**
   This file implements a tree of properties.
   A property is a couple <name, value> where name is a string
   and value is also a string.

   This is used together with a parser to build a tree of properties.
   (See the sysparser.cpp example).

   It is then possible to use the visitor pattern to visit the tree.
*/   
namespace tipa {
    class PropertyLeaf;
    class PropertyNode;

    using PropertyTypeList = std::tuple<PropertyLeaf, PropertyNode>;

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
    class PropertyLeaf : public AbsProperty, public Visitable<PropertyTypeList, PropertyLeaf> {
        std::string value;
    public:
        PropertyLeaf(const std::string &n, const std::string &v) :
            AbsProperty(n), value(v) {}
        std::string get_value() { return value; }
    };

    // A list of abstract properties (intermediate node of the tree)
    class PropertyNode : public AbsProperty, public Visitable<PropertyTypeList, PropertyNode> {
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
