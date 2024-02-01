#include "AST.h"

using namespace ast;

std::ostream& operator<< (std::ostream& os, const AstNode& astNode) {
    astNode.print(os);
    return os;
}

std::ostream& AstNode::print(std::ostream& os) const {
    return os;
}

std::ostream& CompilationUnit::print(std::ostream& os) const {
    os << (*packageDeclaration) << (*importDeclarations) << (*typeDeclarations);
    return os;
}
