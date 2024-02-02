#pragma once

#include "ast/ASTNode.h"

namespace ast {

class VarDecl;
class FieldDecl;
class MethodDecl;

class ClassDecl : public DeclContext, public Decl {
    Modifiers modifiers;
    QualifiedIdentifier* superClass; // Extends super class , null if no extends
    std::vector<QualifiedIdentifier*> interfaces; // Implements interfaces, empty if no implements
    std::vector<FieldDecl*> fields;
    std::vector<MethodDecl*> methods;
    std::vector<MethodDecl*> constructors;
public:
    ClassDecl(
        Modifiers modifiers,
        std::string name,
        QualifiedIdentifier* superClass,
        std::vector<QualifiedIdentifier*> interfaces,
        std::vector<Decl*> classBodyDecls
    ): Decl{name}, modifiers{modifiers}, superClass{superClass}, interfaces{interfaces} {
        // Check classBodyDecls and sort into fields, methods, and constructors
        
    };
};

class InterfaceDecl : public DeclContext, public Decl {
    Modifiers modifiers;
    std::vector<MethodDecl*> methods;
public:
    InterfaceDecl(
        Modifiers modifiers,
        std::string name,
        std::vector<Decl*> interfaceBodyDecls
    ): Decl{name}, modifiers{modifiers} {
        // Check interfaceBodyDecls and sort into methods
    };
};

class MethodDecl : public DeclContext, public Decl {
    Modifiers modifiers;
    Type* returnType;
    std::vector<VarDecl*> parameters;
    bool isConstructor;
    Stmt *body;
public:
    MethodDecl(
        Modifiers modifiers,
        std::string name,
        Type* returnType,
        std::vector<VarDecl*> parameters,
        bool isConstructor,
        Stmt *body
    ) : Decl{name}, modifiers{modifiers}, returnType{returnType}, parameters{parameters}, isConstructor{isConstructor}, body{body} {}
};

} // namespace ast
