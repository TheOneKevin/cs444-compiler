#pragma once

#include "ast/AstNode.h"
#include "diagnostics/Location.h"

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
   VarDecl(BumpAllocator& alloc,
           SourceRange location,
           Type* type,
           string_view name,
           Expr* init) noexcept
         : TypedDecl{alloc, type, name}, init_{init}, location_{location} {}

   bool hasInit() const { return init_ != nullptr; }
   Expr* init() const { return init_; }
   std::ostream& print(std::ostream& os, int indentation = 0) const override;
   int printDotNode(DotPrinter& dp) const override;
   virtual bool hasCanonicalName() const override { return false; }
   auto location() const { return location_; }

private:
   Expr* init_;
   SourceRange location_;
};

class FieldDecl final : public VarDecl {
public:
   FieldDecl(BumpAllocator& alloc,
             SourceRange location,
             Modifiers modifiers,
             Type* type,
             string_view name,
             Expr* init) noexcept
         : VarDecl{alloc, location, type, name, init}, modifiers{modifiers} {};

   std::ostream& print(std::ostream& os, int indentation = 0) const override;
   int printDotNode(DotPrinter& dp) const override;
   bool hasCanonicalName() const override { return true; }

private:
   Modifiers modifiers;
};

} // namespace ast
