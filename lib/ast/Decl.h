#pragma once

#include "ast/AstNode.h"

namespace ast {

class TypedDecl : public Decl {
public:
   TypedDecl(BumpAllocator& alloc, Type* type, string_view name) noexcept
         : Decl{alloc, name}, type_{type} {}
   Type* type() const { return type_; }

private:
   Type* type_;
};

class VarDecl : public TypedDecl {
public:
   VarDecl(BumpAllocator& alloc, Type* type, string_view name, Expr* init) noexcept
         : TypedDecl{alloc, type, name}, init_{init} {}
   bool hasInit() const { return init_ != nullptr; }
   Expr* init() const { return init_; }
   std::ostream& print(std::ostream& os, int indentation = 0) const override;
   int printDotNode(DotPrinter& dp) const override;

private:
   Expr* init_;
};

class FieldDecl : public VarDecl {
public:
   FieldDecl(BumpAllocator& alloc,
             Modifiers modifiers,
             Type* type, string_view name, Expr* init) noexcept
         : VarDecl{alloc, type, name, init}, modifiers{modifiers} {};
   std::ostream& print(std::ostream& os, int indentation = 0) const override;
   int printDotNode(DotPrinter& dp) const override;

private:
   Modifiers modifiers;
};

} // namespace ast
