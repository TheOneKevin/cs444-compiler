#include "ParseTreeTypes.h"
#include <iostream>

using namespace parsetree;

namespace {

int print_dot_recursive(std::ostream& os, const Node& node, int& id_counter) {
    const int id = id_counter++;
    os << "  " << id << " [label=\"";
    node.print_type_and_value(os);
    os << "\"];\n";
    for(size_t i = 0; i < node.num_children(); i++) {
        const Node* child = node.child(i);
        int child_id = 0;
        if(child != nullptr) {
            child_id = print_dot_recursive(os, *child, id_counter);
        } else {
            child_id = id_counter++;
            os << child_id << " [label=\"ε\"];\n";
        }
        os << "  " << id << " -> " << child_id << ";\n";
    }
    return id;
}

};

void parsetree::print_dot(std::ostream& os, const Node& root) {
    os << "digraph G {\n";
    int id_counter = 0;
    print_dot_recursive(os, root, id_counter);
    os << "}\n";
}
