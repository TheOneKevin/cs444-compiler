#pragma once

#include "ast/ASTNode.h"

namespace ast {

class TypedDecl : public Decl {
    Type* type;
public:
    TypedDecl(Type* type, std::string name): Decl{name}, type{type} {}

    std::ostream& print(std::ostream& os) const;
};

class VarDecl : public TypedDecl {
    Stmt* init;
public:
    VarDecl(Type* type, std::string name): TypedDecl{type, name} {

    }
};

class FieldDecl : public VarDecl {
    Modifiers modifiers;
public:
    FieldDecl(
        Modifiers modifiers,
        Type* type,
        std::string name
    ): VarDecl{type, name}, modifiers{modifiers} {};
};

} // namespace ast
