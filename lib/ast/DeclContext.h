#pragma once

#include <ranges>

#include "ast/AstNode.h"
#include "ast/Type.h"
#include "utils/Utils.h"

namespace ast {

class VarDecl;
class FieldDecl;
class MethodDecl;

struct ImportDeclaration {
   ReferenceType* type;
   bool isOnDemand;
   auto simpleName() const {
      auto unresTy = dynamic_cast<UnresolvedType*>(type);
      assert(unresTy && "Can only extract simple name from unresolved type");
      return unresTy->parts().back();
   }
};

class CompilationUnit final : public DeclContext {
public:
   CompilationUnit(BumpAllocator& alloc,
                   ReferenceType* package,
                   array_ref<ImportDeclaration> imports,
                   SourceRange location,
                   DeclContext* body) noexcept;
   auto body() const { return body_; }
   std::ostream& print(std::ostream& os, int indentation = 0) const override;
   int printDotNode(DotPrinter& dp) const override;
   string_view getPackageName() const { 
      if (package_)
         return package_->toString();
      return "unnamed package"; 
   }
   SourceRange location() const { return location_; }
   auto package() const { return package_; }

private:
   ReferenceType* package_;
   pmr_vector<ImportDeclaration> imports_;
   DeclContext* body_;
   SourceRange location_;
};

class LinkingUnit final : public DeclContext{
public:
   LinkingUnit(BumpAllocator& alloc,
               array_ref<CompilationUnit*> compilationUnits) noexcept;
   auto compliationUnits() const { return std::views::all(compilationUnits_); }
   std::ostream& print(std::ostream& os, int indentation = 0) const override;
   int printDotNode(DotPrinter& dp) const override;

private:
   pmr_vector<CompilationUnit*> compilationUnits_;
};

class ClassDecl : public DeclContext, public Decl {
public:
   ClassDecl(BumpAllocator& alloc,
             Modifiers modifiers,
             SourceRange location,
             string_view name,
             ReferenceType* superClass,
             array_ref<ReferenceType*> interfaces,
             array_ref<Decl*> classBodyDecls) throw();
   auto fields() const { return std::views::all(fields_); }
   auto methods() const { return std::views::all(methods_); }
   auto constructors() const { return std::views::all(constructors_); }
   auto interfaces() const { return std::views::all(interfaces_); }
   auto superClass() const { return superClass_; }
   auto modifiers() const { return modifiers_; }
   bool hasCanonicalName() const override { return true; }
   std::ostream& print(std::ostream& os, int indentation = 0) const override;
   int printDotNode(DotPrinter& dp) const override;
   /// @brief Overrides the setParent to construct canonical name.
   void setParent(DeclContext* parent) override;
   SourceRange location() const { return location_; }

private:
   Modifiers modifiers_;
   ReferenceType* superClass_;
   pmr_vector<ReferenceType*> interfaces_;
   pmr_vector<FieldDecl*> fields_;
   pmr_vector<MethodDecl*> methods_;
   pmr_vector<MethodDecl*> constructors_;
   SourceRange location_;
};

class InterfaceDecl : public DeclContext, public Decl {
public:
   InterfaceDecl(BumpAllocator& alloc,
                 Modifiers modifiers,
                 SourceRange location,
                 string_view name,
                 array_ref<ReferenceType*> extends,
                 array_ref<Decl*> interfaceBodyDecls) throw();
   auto extends() const { return std::views::all(extends_); }
   auto methods() const { return std::views::all(methods_); }
   auto modifiers() const { return modifiers_; }
   bool hasCanonicalName() const override { return true; }
   std::ostream& print(std::ostream& os, int indentation = 0) const override;
   int printDotNode(DotPrinter& dp) const override;
   /// @brief Overrides the setParent to construct canonical name.
   void setParent(DeclContext* parent) override;
   SourceRange location() const { return location_; }

private:
   Modifiers modifiers_;
   pmr_vector<ReferenceType*> extends_;
   pmr_vector<MethodDecl*> methods_;
   SourceRange location_;
};

class MethodDecl : public DeclContext, public Decl {
public:
   MethodDecl(BumpAllocator& alloc,
              Modifiers modifiers,
              SourceRange location,
              string_view name,
              Type* returnType,
              array_ref<VarDecl*> parameters,
              bool isConstructor,
              Stmt* body) noexcept
         : Decl{alloc, name},
           modifiers_{modifiers},
           returnType_{returnType},
           parameters_{alloc},
           locals_{alloc},
           isConstructor_{isConstructor},
           body_{body},
           location_{location} {
      utils::move_vector<VarDecl*>(parameters, parameters_);
   }
   auto modifiers() const { return modifiers_; }
   bool isConstructor() const { return isConstructor_; }
   auto parameters() const { return std::views::all(parameters_); }
   bool hasCanonicalName() const override { return true; }
   std::ostream& print(std::ostream& os, int indentation = 0) const override;
   int printDotNode(DotPrinter& dp) const override;
   /// @brief Overrides the setParent to construct canonical name.
   void setParent(DeclContext* parent) override;
   template <std::ranges::range T>
      requires std::same_as<std::ranges::range_value_t<T>, VarDecl*>
   void addDecls(T decls) {
      locals_.reserve(decls.size());
      locals_.insert(locals_.end(), decls.begin(), decls.end());
   }
   SourceRange location() const { return location_; }

private:
   Modifiers modifiers_;
   Type* returnType_;
   pmr_vector<VarDecl*> parameters_;
   pmr_vector<VarDecl*> locals_;
   bool isConstructor_;
   Stmt* body_;
   SourceRange location_;
};

} // namespace ast
