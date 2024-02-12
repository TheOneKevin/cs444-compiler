#include "ParseTreeVisitor.h"
#include <list>

namespace parsetree {

using pty = Node::Type;

std::list<ast::ExprOp> ParseTreeVisitor::visitExpr(parsetree::Node* node) {
    if (node == nullptr) {
        return std::list<ast::ExprOp>();
    }
    

    check_node_type(node, pty::Expression);
}

} // namespace parsetree
