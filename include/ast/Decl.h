#pragma once

#include "ast/AstNode.h"
#include "diagnostics/Location.h"

namespace ast {

/**
 * @brief Represents a variable declaration with: a name, a type and an
 * optional initializer.
 */
class TypedDecl : public Decl {
public:
   TypedDecl(BumpAllocator& alloc, SourceRange location, Type* type,
             string_view name, Expr* init, ScopeID const* scope) noexcept
         : Decl{alloc, name},
           type_{type},
           init_{init},
           location_{location},
           scope_{scope} {}

   Type const* type() const { return type_; }
   Type* mut_type() const { return type_; }
   utils::Generator<ast::AstNode const*> children() const override final {
      co_yield type_;
   }
   bool hasInit() const { return init_ != nullptr; }
   Expr const* init() const { return init_; }
   Expr* mut_init() { return init_; }
   SourceRange location() const override { return location_; }
   ScopeID const* scope() const { return scope_; }

private:
   Type* type_;
   Expr* init_;
   SourceRange location_;
   ScopeID const* const scope_;
};

/**
 * @brief Represents a scoped (i.e., local) typed variable declaration.
 */
class VarDecl : public TypedDecl {
public:
   VarDecl(BumpAllocator& alloc, SourceRange location, Type* type,
           string_view name, Expr* init, ScopeID const* scope) noexcept
         : TypedDecl{alloc, location, type, name, init, scope} {}

   std::ostream& print(std::ostream& os, int indentation = 0) const override;
   int printDotNode(DotPrinter& dp) const override;
   virtual bool hasCanonicalName() const override { return false; }
};

/**
 * @brief Represents a typed declaration with access modifiers.
 */
class FieldDecl final : public TypedDecl {
public:
   FieldDecl(BumpAllocator& alloc, SourceRange location, Modifiers modifiers,
             Type* type, string_view name, Expr* init,
             ScopeID const* scope) noexcept
         : TypedDecl{alloc, location, type, name, init, scope},
           modifiers_{modifiers} {};

   std::ostream& print(std::ostream& os, int indentation = 0) const override;
   int printDotNode(DotPrinter& dp) const override;
   bool hasCanonicalName() const override { return modifiers_.isStatic(); }
   auto modifiers() const { return modifiers_; }
   void setParent(DeclContext* parent) override {
      Decl::setParent(parent);
      auto parentDecl = dyn_cast<Decl>(parent);
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
