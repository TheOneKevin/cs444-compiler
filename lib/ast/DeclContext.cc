#include "ast/AST.h"
#include <ostream>
#include <string>

namespace ast {

using std::ostream;
using std::string;

// CompilationUnit /////////////////////////////////////////////////////////////
    
ostream& CompilationUnit::print(ostream& os, int indentation) const {
    auto i1 = indent(indentation);
    auto i2 = indent(indentation + 1);
    auto i3 = indent(indentation + 2);
    os  << i1 << "CompilationUnit {\n"
        << i2 << "package: " << (package ? package->toString() : "null") << "\n"
        << i2 << "imports: [";
    for (auto& import : imports) {
        os  << "\"" << import.qualifiedIdentifier->toString()
            << (import.isOnDemand ? ".*" : "")
            << "\",";
    }
    os  << "]\n";
    if (body) {
        body->print(os, indentation + 1);
    }
    os << i1 << "}\n";
    return os;
}

// ClassDecl ///////////////////////////////////////////////////////////////////

ClassDecl::ClassDecl(
    Modifiers modifiers,
    std::string name,
    QualifiedIdentifier* superClass,
    std::vector<QualifiedIdentifier*> interfaces,
    std::vector<Decl*> classBodyDecls
): Decl{name}, modifiers{modifiers}, superClass{superClass},
   interfaces{interfaces} {
    // Check that the modifiers are valid for a class
    if (modifiers.isAbstract() && modifiers.isFinal()) {
        throw std::runtime_error("Class cannot be both abstract and final");
    }
    if (!modifiers.isPublic()) {
        throw std::runtime_error("Class must have a visibility modifier");
    }
    // Sort the classBodyDecls into fields, methods, and constructors
    for(auto bodyDecl : classBodyDecls) {
        if (auto fieldDecl = dynamic_cast<FieldDecl*>(bodyDecl)) {
            fields.push_back(fieldDecl);
        } else if (auto methodDecl = dynamic_cast<MethodDecl*>(bodyDecl)) {
            if(methodDecl->isConstructor()) {
                constructors.push_back(methodDecl);
            } else {
                methods.push_back(methodDecl);
            }
        } else {
            throw std::runtime_error("Invalid class body declaration");
        }
    }

    if (constructors.size() == 0) {
        throw std::runtime_error("A class must have at least one constructor");
    }
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

// InterfaceDecl ///////////////////////////////////////////////////////////////

InterfaceDecl::InterfaceDecl(
    Modifiers modifiers,
    std::string name,
    std::vector<QualifiedIdentifier*> extends,
    std::vector<Decl*> interfaceBodyDecls
): Decl{name}, modifiers{modifiers}, extends{extends} {
    if (modifiers.isFinal()) {
        throw std::runtime_error("An interface cannot be final");
    }
    if (!modifiers.isPublic()) {
        throw std::runtime_error("Interface must have a visibility modifier");
    }
    for(auto bodyDecl : interfaceBodyDecls) {
        if (auto methodDecl = dynamic_cast<MethodDecl*>(bodyDecl)) {
            if(!methodDecl->getModifiers().isAbstract()) {
                throw std::runtime_error("An interface can only contain abstract methods");
            }
            methods.push_back(methodDecl);
        } else {
            throw std::runtime_error("Invalid interface body declaration");
        }
    }
}

ostream& InterfaceDecl::print(ostream& os, int indentation) const {
    auto i1 = indent(indentation);
    auto i2 = indent(indentation + 1);
    auto i3 = indent(indentation + 2);
    os  << i1 << "InterfaceDecl {\n"
        << i2 << "modifiers: " << modifiers.toString() << "\n"
        << i2 << "name: " << this->getName() << "\n"
        << i2 << "extends: []\n"
        << i2 << "methods:\n";
    for (auto& method : methods) {
        method->print(os, indentation + 2);
    }
    os << i1 << "}\n";
    return os;
}

// MethodDecl //////////////////////////////////////////////////////////////////

MethodDecl::MethodDecl(
    Modifiers modifiers,
    std::string name,
    Type* returnType,
    std::vector<VarDecl*> parameters,
    bool isConstructor,
    Stmt *body
) : Decl{name}, modifiers{modifiers}, returnType{returnType},
    parameters{parameters}, isConstructor_{isConstructor}, body{body} {
    // Check modifiers
    if ((body == nullptr) != (modifiers.isAbstract() || modifiers.isNative())) {
        throw std::runtime_error("A method has a body if and only if it is neither abstract nor native. " + name);
    }
    if (modifiers.isAbstract() && (modifiers.isFinal() || modifiers.isStatic() || modifiers.isNative())) {
        throw std::runtime_error("An abstract method cannot be static, final or native. " + name);
    }
    if (modifiers.isStatic() && modifiers.isFinal()) {
        throw std::runtime_error("A static method cannot be final. " + name);
    }
    if (modifiers.isNative() && !modifiers.isStatic()) {
        throw std::runtime_error("A native method must be static. " + name);
    }
    if (modifiers.isPublic() && modifiers.isProtected()) {
        throw std::runtime_error("A method cannot be both public and protected. " + name);
    }
    if (!modifiers.isPublic() && !modifiers.isProtected()) {
        throw std::runtime_error("Method must have a visibility modifier");
    }
}
    
ostream& MethodDecl::print(ostream& os, int indentation) const {
    auto i1 = indent(indentation);
    auto i2 = indent(indentation + 1);
    auto i3 = indent(indentation + 2);
    os  << i1 << "MethodDecl {\n"
        << i2 << "modifiers: " << modifiers.toString() << "\n"
        << i2 << "name: " << this->getName() << "\n"
        << i2 << "returnType: " << (returnType ? returnType->toString() : "null") << "\n"
        << i2 << "parameters:\n";
    for (auto& parameter : parameters) {
        parameter->print(os, indentation + 2);
    }
    if (body) {
        body->print(os, indentation + 1);
    }
    os << i1 << "}\n";
    return os;
}

} // namespace ast
