#include "ParseTreeVisitor.h"

namespace parsetree {

using pty = Node::Type;

std::list<ast::ExprOp> visitExpr(parsetree::Node* node) {
    if (node == nullptr) {
        return list<ast::ExprOp>();
    }
    

    check_node_type(node, pty::Expression);
}

} // namespace parsetree
