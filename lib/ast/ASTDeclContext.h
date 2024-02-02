#pragma once

#include "AST.h"

namespace ast {

class Modifiers {
    bool isPublic;
    bool isProtected;
    bool isStatic;
    bool isFinal;
    bool isAbstract;
    bool isNative;
public:
};

class ClassDeclaration : public DeclContext, public Decl {
    Modifiers modifiers;
    QualifiedIdentifier* superClass; // Extends super class , null if no extends
    std::vector<QualifiedIdentifier*> interfaces; // Implements interfaces, empty if no implements
    std::vector<FieldDeclaration*> fields;
    std::vector<MethodDeclaration*> methods;
    std::vector<MethodDeclaration*> constructors;
public:
    ClassDeclaration(
        Modifiers modifiers,
        std::string name,
        QualifiedIdentifier* superClass,
        std::vector<QualifiedIdentifier*> interfaces,
        std::vector<Decl*> classBodyDeclarations
    ): Decl{name}, modifiers{modifiers}, identifier{identifier}, superClass{superClass}, interfaces{interfaces} {
        // Check classBodyDeclarations and sort into fields, methods, and constructors
        
    };
};

class InterfaceDeclaration : public DeclContext, public Decl {
    Modifiers modifiers;
    std::vector<MethodDeclaration*> methods;
public:
    InterfaceDeclaration(
        Modifiers modifiers,
        std::string name,
        std::vector<Decl*> interfaceBodyDeclarations
    ): Decl{name}, modifiers{modifiers} {
        // Check interfaceBodyDeclarations and sort into methods
    };
};

class MethodDeclaration : public DeclContext, public Decl {
    Modifiers modifiers;
    Type* returnType;
    std::vector<VariableDeclaration*> parameters;
    bool isConstructor;
    Stmt *body;
public:
    MethodDeclaration(
        Modifiers modifiers,
        std::string name,
        Type* returnType,
        std::vector<VariableDeclaration*> parameters,
        bool isConstructor,
        Stmt *body
    ) : Decl{name}, modifiers{modifiers}, returnType{returnType}, parameters{parameters}, isConstructor{isConstructor}, body{body} {}
};

} // namespace ast
