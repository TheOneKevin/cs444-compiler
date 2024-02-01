#include "AST.h"

using namespace ast;

std::ostream& ast::operator<< (std::ostream& os, const AstNode& astNode) {
    astNode.print(os);
    return os;
}

std::ostream& AstNode::print(std::ostream& os) const {
    return os;
}

std::ostream& CompilationUnit::print(std::ostream& os) const {
    os << "(CompilationUnit: ";
    if (packageDeclaration != nullptr) os << (*packageDeclaration);
    if (importDeclarations != nullptr) os << (*importDeclarations);
    if (typeDeclarations != nullptr) os << (*typeDeclarations);
    os << ")";
    return os;
}

std::ostream& PackageDeclaration::print(std::ostream& os) const {
    os << "(PackageDeclaration: ";
    os << (*qualifiedIdentifier);
    os << ")";
    return os;
}

std::ostream& ImportDeclarations::print(std::ostream& os) const {
    os << "(ImportDeclarations: ";
    for (auto ptr = imports.begin(); ptr != imports.end();) {
        os << (**ptr);
        if (++ptr == imports.end()) break;
        os << ",";
    }
    os << ")";
    return os;
}

std::ostream& Import::print(std::ostream& os) const {
    os << (*qualifiedIdentifier);
    if (!isSingleType) os << ".*";
    return os;
}


std::ostream& QualifiedIdentifier::print(std::ostream& os) const {
    os << "(QualifiedIdentifier: ";
    for (auto ptr = identifiers.begin(); ptr != identifiers.end();) {
        os << (*(*ptr));
        if (++ptr == identifiers.end()) break;
        os << ".";
    }
    os << ")";
    return os;
}

std::ostream& Identifier::print(std::ostream& os) const {
    os << value;
    return os;
}
