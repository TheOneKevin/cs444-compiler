#pragma once

#include "ast/AstNode.h"

namespace ast {

class TypedDecl : public Decl {
    Type* type;
public:
    TypedDecl(Type* type, std::string name): Decl{name}, type{type} {}
};

class VarDecl : public TypedDecl {
    Stmt* init;
public:
    VarDecl(Type* type, std::string name): TypedDecl{type, name} {

    }
    std::ostream& print(std::ostream& os, int indentation = 0) const override;
};

class FieldDecl : public VarDecl {
    Modifiers modifiers;
public:
    FieldDecl(
        Modifiers modifiers,
        Type* type,
        std::string name
    ): VarDecl{type, name}, modifiers{modifiers} {};
    std::ostream& print(std::ostream& os, int indentation = 0) const override;
};

} // namespace ast
