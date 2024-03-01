#pragma once

#include "ast/AstNode.h"
#include "diagnostics/Location.h"

namespace ast {

class TypedDecl : public Decl {
public:
   TypedDecl(BumpAllocator& alloc, Type* type, string_view name) noexcept
         : Decl{alloc, name}, type_{type} {}

   Type* type() const { return type_; }
   Type* mut_type() const { return type_; }
   utils::Generator<ast::AstNode const*> children() const override final {
      co_yield type_;
   }

private:
   Type* type_;
};

class VarDecl : public TypedDecl {
public:
   VarDecl(BumpAllocator& alloc, SourceRange location, Type* type,
           string_view name, Expr* init) noexcept
         : TypedDecl{alloc, type, name}, init_{init}, location_{location} {}

   bool hasInit() const { return init_ != nullptr; }
   Expr const* init() const { return init_; }
   Expr* mut_init() { return init_; }
   std::ostream& print(std::ostream& os, int indentation = 0) const override;
   int printDotNode(DotPrinter& dp) const override;
   virtual bool hasCanonicalName() const override { return false; }
   SourceRange location() const override { return location_; }

private:
   Expr* init_;
   SourceRange location_;
};

class FieldDecl final : public VarDecl {
public:
   FieldDecl(BumpAllocator& alloc, SourceRange location, Modifiers modifiers,
             Type* type, string_view name, Expr* init) noexcept
         : VarDecl{alloc, location, type, name, init}, modifiers_{modifiers} {};

   std::ostream& print(std::ostream& os, int indentation = 0) const override;
   int printDotNode(DotPrinter& dp) const override;
   bool hasCanonicalName() const override { return modifiers_.isStatic(); }
   auto modifiers() const { return modifiers_; }
   void setParent(DeclContext* parent) override {
      Decl::setParent(parent);
      Decl* parentDecl = dynamic_cast<Decl*>(parent);
      assert(parentDecl && "Parent must be a Decl");
      if(modifiers_.isStatic()) {
         canonicalName_ = parentDecl->getCanonicalName();
         canonicalName_ += ".";
         canonicalName_ += name();
      }
   }

private:
   Modifiers modifiers_;
};

} // namespace ast
