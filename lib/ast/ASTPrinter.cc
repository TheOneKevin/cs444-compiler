#include "ast/ASTNode.h"

using namespace ast;

std::ostream& ast::operator<< (std::ostream& os, const AstNode& astNode) {
    astNode.print(os);
    return os;
}

std::ostream& AstNode::print(std::ostream& os) const {
    return os;
}
