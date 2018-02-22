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

  -----------------------------------------------------------------

  This is an implementation of the Visitor pattern in a modern C++
  style.

  How to use:

  First, we need to define a tuple of all types of visitable objects,
  let's call it TypeList:

      using TypeList = std::tuple<Mesh, Text>;

  Then each class must derive from Visitable<TypeList, ClassType>, i.e.

      class Mesh : public ..., public Visitable<TypeList, Mesh> {};
      class Text : public ..., public Visitable<TypeList, Text> {};

  Then the visitor: 

      class MyVisitor : public Visitor<TypeList> {
      public:
          void visit(Mesh &m) { ... }
          void visit(Text &t) { ... }
      };

  Of course, at the highest level, you should call accept() by passing
  the visitor object.
    
  Notice that adding a new class in the hierarchy requires to modify
  the type list and recompile everything.

  You sometimes need to have a hierarchy of classes that are similar
  to each other. In this case the root of the hierarchy (usually an
  abstract class) is not part of the visitable set of classes, in fact
  we will never actually need to visit the abstract class (there is no
  object of that type). Also, we need a lot of dynamic casts here and
  there, because to call accept wee need to first cast to
  AbstractVisitable<TypeList>. That's the price to pay to be generic!

  For example, if Mesh and Text both derive from abstract class Foo,
  then Foo should not derive from Visitable. And if you have a pointer
  to Foo, you must cast it before calling accept. 

      Foo *foo = ...
      dynamic_cast<AbstractVisitable<TypeList> *>(foo)->accept(visitor);

  Therefore, I wrote the helper function accept, so the above becomes:

      Foor *foo = ...
      accept<TypeList>(*foo, visitor);

  The first parameter can be pretty much anything, and if the dynamic
  cast fails, a bad_cast exception is raised. You can also pass a shared_ptr<>.

  (TODO: extend to any kind of reference or pointer).
*/


#ifndef __GEN_VISITOR_HPP__
#define __GEN_VISITOR_HPP__

#include <utility>
#include <memory>
#include <exception>

namespace tipa {
    /* 
       This is just the most generic template version. Several
       specialization will follow. We expect X to be a tuple of types
       to visit.
     */
    template<typename X>
    struct Visitor {};

    // End of recursion on the typelist
    // When there is no more types left to analyse
    template<>
    struct Visitor<std::tuple<>> {};

    // The visit method for each type
    // This is a trick to have virtual functions for every type
    template<typename T>
    struct _virtualvisitmethod {
        virtual void visit(T&) = 0;
    };

    // Recursive template, each one implements the
    // visit method on the first type of the list. 
    template<typename T, typename... TS>
    struct Visitor<std::tuple<T, TS...>> :
        Visitor<std::tuple<TS...>>, _virtualvisitmethod<T> {};

    /*
      This must be an abstract visitable object. 
     */
    template<typename TYPELIST>
    struct AbstractVisitable {
        virtual void accept(Visitor<TYPELIST>&) = 0;
    };
    

    /**
       This is the base class of every visitable object. It takes two
       parameters: the list of all types of visitable objects, and the
       type of this particular object.
     */
    template<typename TYPELIST, typename T>
    struct Visitable : AbstractVisitable<TYPELIST> {
        virtual void accept(Visitor<TYPELIST>& v) override {
            auto &vv = static_cast< _virtualvisitmethod<T>& >(v);
            vv.visit(*static_cast<T*>(this));
        }
    };
    
    template<typename TYPELIST, typename C, typename V>
    void accept(C &obj, V& visitor) {
        dynamic_cast<AbstractVisitable<TYPELIST> &>(obj).accept(visitor);
    }

    template<typename TYPELIST, typename C, typename V>
    void accept(std::shared_ptr<C> obj, V& visitor) {
        auto p = std::dynamic_pointer_cast<AbstractVisitable<TYPELIST> >(obj);
        if (p == nullptr) throw std::bad_cast();
        p->accept(visitor);
    }
}


#endif
