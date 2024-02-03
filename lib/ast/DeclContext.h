#pragma once

#include "AstNode.h"

namespace ast {

class VarDecl;
class FieldDecl;
class MethodDecl;

struct ImportDeclaration {
    QualifiedIdentifier* qualifiedIdentifier;
    bool isOnDemand;
};

class CompilationUnit final : public DeclContext {
    QualifiedIdentifier* package;
    std::vector<ImportDeclaration> imports;
    DeclContext* body;
public:
    CompilationUnit(
        QualifiedIdentifier* package,
        std::vector<ImportDeclaration> imports,
        DeclContext* body
    ) : package{package}, imports{imports}, body{body} {}
    std::ostream& print(std::ostream& os, int indentation = 0) const override;
    DeclContext* getBody() { return body; }
};

class ClassDecl : public DeclContext, public Decl {
    Modifiers modifiers;
    QualifiedIdentifier* superClass;
    std::vector<QualifiedIdentifier*> interfaces;
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
    );
    
    std::ostream& print(std::ostream& os, int indentation = 0) const override;
};

class InterfaceDecl : public DeclContext, public Decl {
    Modifiers modifiers;
    std::vector<QualifiedIdentifier*> extends;
    std::vector<MethodDecl*> methods;
public:
    InterfaceDecl(
        Modifiers modifiers,
        std::string name,
        std::vector<QualifiedIdentifier*> extends,
        std::vector<Decl*> interfaceBodyDecls
    );
    std::ostream& print(std::ostream& os, int indentation = 0) const override;
};

class MethodDecl : public DeclContext, public Decl {
    Modifiers modifiers;
    Type* returnType;
    std::vector<VarDecl*> parameters;
    bool isConstructor_;
    Stmt *body;
public:
    MethodDecl(
        Modifiers modifiers,
        std::string name,
        Type* returnType,
        std::vector<VarDecl*> parameters,
        bool isConstructor,
        Stmt *body
    );
    std::ostream& print(std::ostream& os, int indentation = 0) const override;
    Modifiers& getModifiers() { return modifiers; }
    bool isConstructor() const { return isConstructor_; }
};

} // namespace ast
