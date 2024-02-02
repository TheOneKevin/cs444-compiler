#include "ast/AST.h"
#include "parsetree/ParseTreeTypes.h"
#include "parsetree/ParseTreeVisitor.h"

namespace parsetree {

using pty = Node::Type;

// pty::Block //////////////////////////////////////////////////////////////////

ast::Stmt* visitBlock(Node* node) {
    check_node_type(node, pty::Block);
    check_num_children(node, 1, 1);
    if(node->child(0) == nullptr)
        return nullptr;
    // FIXME(kevin): Implement this
    return nullptr;
}

} // namespace parsetree
