#pragma once

#include <iostream>

#include "parsetree/ParseTree.h"

namespace jcc1 {

enum class InputMode { File, Stdin };

/// @brief Prints the parse tree back to the parent node
/// @param node Node to trace back to the parent
inline void trace_node(parsetree::Node const* node, std::ostream& os = std::cerr) {
   if(node->parent() != nullptr) {
      trace_node(node->parent());
      os << " -> ";
   }
   os << node->type_string() << std::endl;
}

/// @brief Marks a node and all its parents
/// @param node The node to mark
inline void mark_node(parsetree::Node* node) {
   if(!node) return;
   mark_node(node->parent());
   node->mark();
}

} // namespace
