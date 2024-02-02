#pragma once

#include <string>
#include <vector>
#include <iostream>
#include "parsetree/ParseTreeTypes.h"

namespace ast {
    
// Base class for all AST nodes
class Type;
class Decl;
class DeclContext;
class Stmt;

class Modifiers {
    bool isPublic;
    bool isProtected;
    bool isStatic;
    bool isFinal;
    bool isAbstract;
    bool isNative;
public:
    void set(parsetree::Modifier modifier) {}
    void set(ast::Modifiers modifier) {}
};

class AstNode {
public:
    virtual std::ostream& print(std::ostream& os) const;
};

class Decl : public AstNode {
    std::string name;
public:
    Decl(std::string name): name{name} {}

    std::ostream& print(std::ostream& os) const;
};

class DeclContext : public AstNode {};

class Type : public AstNode {};

class Stmt : public AstNode {
    
};

struct QualifiedIdentifier {
    std::vector<std::string> identifiers;
    void add_identifier(std::string identifier) {
        identifiers.push_back(identifier);
    }
};

struct ImportDeclaration {
    QualifiedIdentifier* qualifiedIdentifier;
    bool isOnDemand;
};

class CompilationUnit final : public DeclContext {
    QualifiedIdentifier* packageDeclaration;
    std::vector<ImportDeclaration> imports;
    DeclContext* typeDeclaration;
public:
    CompilationUnit(
        QualifiedIdentifier* packageDeclaration,
        std::vector<ImportDeclaration> imports,
        DeclContext* typeDeclaration
    ) : packageDeclaration{packageDeclaration}, imports{imports}, typeDeclaration{typeDeclaration} {}

    std::ostream& print(std::ostream& os) const;
};

std::ostream& operator<< (std::ostream& os, const AstNode& astNode);

} // namespace ast
