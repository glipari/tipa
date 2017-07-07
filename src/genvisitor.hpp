#ifndef __GEN_VISITOR_HPP__
#define __GEN_VISITOR_HPP__

#include <utility>

namespace tipa {

    template<typename>
    struct Visitor {};

    // end of recursion
    template<>
    struct Visitor<std::tuple<>> {};

    // the visit method for each type
    template<typename T>
    struct _virtualvisitmethod {
        virtual void visit(T&) = 0;
    };

    // recursive template
    template<typename T, typename... TS>
    struct Visitor<std::tuple<T, TS...>> :
        Visitor<std::tuple<TS...>>, _virtualvisitmethod<T> {};

    
    template<typename TYPELIST>
    struct AbstractVisitable {
        virtual void accept(Visitor<TYPELIST>&) = 0;
    };
    
    
    template<typename TYPELIST, typename T>
    struct Visitable : AbstractVisitable<TYPELIST> {
        virtual void accept(Visitor<TYPELIST>& v) override {
            static_cast<_virtualvisitmethod<T>&>(v).visit(*static_cast<T*>(this));
        }
    };
    
    
    //  // Visitor template declaration
    //  template<typename... Types>
    //  class Visitor;

    //  // specialization for single type    
    //  template<typename T>
    //  class Visitor<T> {
    //  public:
    //      virtual void visit(T & visitable) = 0;
    //  };

    //  // specialization for multiple types
    //  template<typename T, typename... Types>
    //  class Visitor<T, Types...> : public Visitor<Types...> {
    //  public:
    //      // promote the function(s) from the base class
    //      using Visitor<Types...>::visit;

    //      virtual void visit(T & visitable) = 0;
    //  };

    //  template<typename... Types>
    //  class Visitable {
    //  public:
    //      virtual void accept(Visitor<Types...>& visitor) = 0;
    //  };


    // template<typename Derived, typename... Types>
    // class VisitableImpl : public Visitable<Types...> {
    // public:
    //     virtual void accept(Visitor<Types...>& visitor) {
    //         visitor.visit(dynamic_cast<Derived&>(*this));
    //     }
    // };


    // template<typename Base, typename Visitor, typename Visitable>
    // void visit(Base &b, Visitor &v) {
    //     Visitable &p = dynamic_cast<Visitable &>(b);
    //     p.accept(v);
    // }

    
    /** How to use:

          class Mesh : public Object, public VisitableImpl<Mesh, Mesh, Text> {};
          class Text : public Object, public VisitableImpl<Text, Mesh, Text> {};

        The first type in the typelist is the name of the class, from
        the second on you have the list classes in the
        hierarchy. 

        Notice that adding a new class in the hierarchy requires to
        modify the type list and recompile everything.
        
        Now the visitor:

          class Renderer : public Visitor<Mesh, Text> {};
        
        If you have a container of "Object"s to visit, you need to cast to
        Visitable<Mesh, Text>&   before invoking the visit function.
     */
}


#endif
