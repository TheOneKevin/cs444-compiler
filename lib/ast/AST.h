#pragma once

#include <string>
#include <vector>
#include <iostream>

namespace ast {
    
// Base class for all AST nodes
class Type;
class Decl;
class DeclContext;
class Stmt;

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
};

/* ========================================================================== */
/*                               Literals                                     */
/* ========================================================================== */

class CompilationUnit final : public DeclContext {
    QualifiedIdentifier packageDeclaration;
    std::vector<QualifiedIdentifier*> imports;
    DeclContext* typeDeclaration;
public:
    CompilationUnit(
        QualifiedIdentifier packageDeclaration,
        std::vector<QualifiedIdentifier*> imports,
        DeclContext* typeDeclaration
    ) : packageDeclaration{packageDeclaration}, imports{imports}, typeDeclaration{typeDeclaration} {}

    std::ostream& print(std::ostream& os) const;
};

std::ostream& operator<< (std::ostream& os, const AstNode& astNode);

} // namespace ast
