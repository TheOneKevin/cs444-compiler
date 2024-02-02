#include "ast/AST.h"
#include <ostream>
#include <string>

namespace ast {

using std::ostream;
using std::string;

ClassDecl::ClassDecl(
    Modifiers modifiers,
    std::string name,
    QualifiedIdentifier* superClass,
    std::vector<QualifiedIdentifier*> interfaces,
    std::vector<Decl*> classBodyDecls
): Decl{name}, modifiers{modifiers}, superClass{superClass}, interfaces{interfaces} {
    // Check that the modifiers are valid for a class
    if (!modifiers.isValidClass()) {
        throw std::runtime_error("Invalid modifiers for class");
    }

    // Check classBodyDecls are sort into fields, methods, and constructors
};
    
ostream& CompilationUnit::print(ostream& os, int indentation) const {
    auto i1 = indent(indentation);
    auto i2 = indent(indentation + 1);
    auto i3 = indent(indentation + 2);
    os  << i1 << "CompilationUnit {\n"
        << i2 << "package: " << package->toString() << "\n"
        << i2 << "imports: [";
    for (auto& import : imports) {
        os  << "\"" << import.qualifiedIdentifier->toString()
            << (import.isOnDemand ? "*" : "")
            << "\",";
    }
    os  << "]\n";
    if (body) {
        body->print(os, indentation + 1);
    }
    os << i1 << "}\n";
    return os;
}

ostream& ClassDecl::print(ostream& os, int indentation) const {
    auto i1 = indent(indentation);
    auto i2 = indent(indentation + 1);
    auto i3 = indent(indentation + 2);
    os  << i1 << "ClassDecl {\n"
        << i2 << "modifiers: " << modifiers.toString() << "\n"
        << i2 << "name: " << this->getName() << "\n"
        << i2 << "superClass: " << (superClass ? superClass->toString() : "null") << "\n"
        << i2 << "interfaces: []\n"
        << i2 << "fields:\n";
    for (auto& field : fields) {
        field->print(os, indentation + 2);
    }
    os << i2 << "methods:\n";
    for (auto& method : methods) {
        method->print(os, indentation + 2);
    }
    os << i2 << "constructors:\n";
    for (auto& constructor : constructors) {
        constructor->print(os, indentation + 2);
    }
    os << i1 << "}\n";
    return os;
}

ostream& InterfaceDecl::print(ostream& os, int indentation) const {
    return os;
}

ostream& MethodDecl::print(ostream& os, int indentation) const {
    return os;
}

} // namespace ast
