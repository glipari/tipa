#include "property.hpp"

using namespace tipa;
using namespace std;

void PropertyBuilder::build_leaf(parser_context &pc)
{
    //cout << "Builder: leaf has been found!" << endl;
    auto v = pc.collect_tokens(2);
    //for (auto t : v) cout << "Builder: Token : " << t.second << endl;
    if (v.size() < 2) throw string("Error in parsing leaf node");
    auto node = make_shared<PropertyLeaf>(PropertyLeaf(v[0].second, v[1].second));
    st.push(node);
}

void PropertyBuilder::build_root_begin(parser_context &pc)
{
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

void PropertyBuilder::build_root_end(parser_context &pc)
{
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
