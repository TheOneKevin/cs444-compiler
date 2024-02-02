#pragma once

#include "ast/AstNode.h"

namespace ast {

class TypedDecl : public Decl {
    Type* type;
public:
    TypedDecl(Type* type, std::string name): Decl{name}, type{type} {}
    Type* getType() const { return type; }
};

class VarDecl : public TypedDecl {
    Stmt* init;
public:
    VarDecl(Type* type, std::string name): TypedDecl{type, name} {

    }
    bool hasInit() const { return init != nullptr; }
    Stmt* getInit() const { return init; }
    std::ostream& print(std::ostream& os, int indentation = 0) const override;
};

class FieldDecl : public VarDecl {
    Modifiers modifiers;
public:
    FieldDecl(
        Modifiers modifiers,
        Type* type,
        std::string name
    ): VarDecl{type, name}, modifiers{modifiers} {
        if (modifiers.isFinal()) {
            throw std::runtime_error("FieldDecl cannot be final");
        }
        if (modifiers.isAbstract()) {
            throw std::runtime_error("FieldDecl cannot be abstract");
        }
        if (modifiers.isNative()) {
            throw std::runtime_error("FieldDecl cannot be native");
        }
        if (modifiers.isPublic() && modifiers.isProtected()) {
            throw std::runtime_error("A method cannot be both public and protected. " + name);
        }
    };
    std::ostream& print(std::ostream& os, int indentation = 0) const override;
};

} // namespace ast
