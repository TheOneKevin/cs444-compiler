#include "ast/AST.h"
#include "parsetree/ParseTree.h"
#include "parsetree/ParseTreeVisitor.h"

namespace parsetree {

using pty = Node::Type;
using ptv = ParseTreeVisitor;

// pty::Block //////////////////////////////////////////////////////////////////

ast::Stmt* ptv::visitBlock(Node* node) {
   check_node_type(node, pty::Block);
   check_num_children(node, 1, 1);
   if(node->child(0) == nullptr) return new ast::CompoundStmt{};
   // FIXME(kevin): Implement this
   return new ast::CompoundStmt{};
}

} // namespace parsetree
