#pragma once

#include <ranges>

#include "ast/AstNode.h"
#include "ast/Decl.h"
#include "ast/Type.h"
#include "utils/Generator.h"
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
   SourceRange location() const { return type->location(); }
};

class CompilationUnit final : public DeclContext {
public:
   CompilationUnit(BumpAllocator& alloc, ReferenceType* package,
                   array_ref<ImportDeclaration> imports, SourceRange location,
                   DeclContext* body) noexcept;
   auto const* body() const { return body_; }
   auto const* bodyAsDecl() const { return dynamic_cast<Decl const*>(body_); }
   auto mut_bodyAsDecl() { return dynamic_cast<Decl*>(body_); }
   std::ostream& print(std::ostream& os, int indentation = 0) const override;
   int printDotNode(DotPrinter& dp) const override;
   string_view getPackageName() const {
      auto package = dynamic_cast<UnresolvedType*>(package_);
      assert(package_ && "Package must be unresolved type");
      if(package->parts().size() > 0) return package_->toString();
      return "unnamed package";
   }
   SourceRange location() const { return location_; }
   auto const* package() const { return package_; }
   auto imports() const { return std::views::all(imports_); }
   auto isDefaultPackage() const {
      auto unresTy = dynamic_cast<UnresolvedType*>(package_);
      assert(unresTy && "Package must be unresolved type");
      return unresTy->parts().size() == 0;
   }
   utils::Generator<ast::AstNode const*> children() const override {
      co_yield package_;
      for(auto import : imports_) co_yield import.type;
      co_yield body_;
   }
   auto mut_body() { return body_; }
   bool isStdLib() const {
      auto package = dynamic_cast<UnresolvedType*>(package_);
      assert(package_ && "Package must be unresolved type");
      return package->parts().size() >= 1 && package->parts().front() == "java";
   }

private:
   ReferenceType* package_;
   pmr_vector<ImportDeclaration> imports_;
   DeclContext* body_;
   SourceRange location_;
};

class LinkingUnit final : public DeclContext {
public:
   LinkingUnit(BumpAllocator& alloc,
               array_ref<CompilationUnit*> compilationUnits) noexcept;
   auto compliationUnits() const { return std::views::all(compilationUnits_); }
   std::ostream& print(std::ostream& os, int indentation = 0) const override;
   int printDotNode(DotPrinter& dp) const override;
   utils::Generator<ast::AstNode const*> children() const override {
      for(auto cu : compilationUnits_) co_yield cu;
   }

private:
   pmr_vector<CompilationUnit*> compilationUnits_;
};

class ClassDecl final : public DeclContext, public Decl {
public:
   ClassDecl(BumpAllocator& alloc, Modifiers modifiers, SourceRange location,
             string_view name, ReferenceType* super1, ReferenceType* super2,
             array_ref<ReferenceType*> interfaces,
             array_ref<Decl*> classBodyDecls) throw();
   auto fields() const { return std::views::all(fields_); }
   auto methods() const { return std::views::all(methods_); }
   auto inheritedMethods() const { return std::views::all(inheritedMethods_); }
   auto constructors() const { return std::views::all(constructors_); }
   auto interfaces() const { return std::views::all(interfaces_); }
   /// @brief Grabs a view of the super classes.
   /// Warning: the super classes may be null.
   auto superClasses() const { return std::views::all(superClasses_); }
   auto& mut_superClasses() { return superClasses_; }
   auto modifiers() const { return modifiers_; }
   bool hasCanonicalName() const override { return true; }
   std::ostream& print(std::ostream& os, int indentation = 0) const override;
   int printDotNode(DotPrinter& dp) const override;
   /// @brief Overrides the setParent to construct canonical name.
   void setParent(DeclContext* parent) override;
   SourceRange location() const override { return location_; }

   /// @brief Set the inherited methods. It should only be called once.
   void setInheritedMethods(std::pmr::vector<MethodDecl*>& inheritedMethods) {
      assert(!inheritedSet_ && "Inherited methods already set");
      inheritedMethods_ = inheritedMethods;
      inheritedSet_ = true;
   }
   bool isInheritedSet() const { return inheritedSet_; }

   utils::Generator<ast::AstNode const*> children() const override {
      for(auto field : fields_) co_yield field;
      for(auto method : methods_) co_yield reinterpret_cast<Decl*>(method);
      for(auto constructor : constructors_)
         co_yield reinterpret_cast<Decl*>(constructor);
      for(auto interface : interfaces_) co_yield interface;
      for(auto superClass : superClasses_) co_yield superClass;
   }

private:
   Modifiers modifiers_;
   std::array<ReferenceType*, 2> superClasses_;
   pmr_vector<ReferenceType*> interfaces_;
   pmr_vector<FieldDecl*> fields_;
   pmr_vector<MethodDecl*> methods_;
   pmr_vector<MethodDecl*> constructors_;
   SourceRange location_;
   pmr_vector<MethodDecl*> inheritedMethods_;
   bool inheritedSet_ = false;
};

class InterfaceDecl final : public DeclContext, public Decl {
public:
   InterfaceDecl(BumpAllocator& alloc, Modifiers modifiers, SourceRange location,
                 string_view name, array_ref<ReferenceType*> extends,
                 ReferenceType* objectSuperclass,
                 array_ref<Decl*> interfaceBodyDecls) throw();
   auto extends() const { return std::views::all(extends_); }
   auto methods() const { return std::views::all(methods_); }
   auto inheritedMethods() const { return std::views::all(inheritedMethods_); }
   auto modifiers() const { return modifiers_; }
   auto const* objectSuperclass() const { return objectSuperclass_; }
   bool hasCanonicalName() const override { return true; }
   std::ostream& print(std::ostream& os, int indentation = 0) const override;
   int printDotNode(DotPrinter& dp) const override;
   /// @brief Overrides the setParent to construct canonical name.
   void setParent(DeclContext* parent) override;
   /// @brief Set the inherited methods. It should only be called once.
   void setInheritedMethods(std::pmr::vector<MethodDecl*>& inheritedMethods) {
      assert(!inheritedSet_ && "Inherited methods already set");
      inheritedMethods_ = inheritedMethods;
      inheritedSet_ = true;
   }
   bool isInheritedSet() const { return inheritedSet_; }
   SourceRange location() const override { return location_; }

   utils::Generator<ast::AstNode const*> children() const override {
      for(auto method : methods_) co_yield reinterpret_cast<Decl*>(method);
      for(auto superClass : extends_) co_yield superClass;
   }

private:
   Modifiers modifiers_;
   pmr_vector<ReferenceType*> extends_;
   pmr_vector<MethodDecl*> methods_;
   SourceRange location_;
   pmr_vector<MethodDecl*> inheritedMethods_;
   ReferenceType* objectSuperclass_;
   bool inheritedSet_ = false;
};

class MethodDecl final : public DeclContext, public Decl {
public:
   /// @brief Represents the return type of a method. This wraps ast::Type
   /// to allow for void return types (we don't consider void a type).
   struct ReturnType {
      const ast::Type* const type;
      bool operator==(ReturnType const& other) const {
         // If one or both are null, then they are equal if both are null.
         if(type == nullptr || other.type == nullptr) return type == other.type;
         // Otherwise, dereference and compare.
         return *type == *other.type;
      }

   private:
      ReturnType(ast::Type const* type) : type{type} {}
      friend class MethodDecl;
   };

public:
   MethodDecl(BumpAllocator& alloc, Modifiers modifiers, SourceRange location,
              string_view name, Type* returnType, array_ref<VarDecl*> parameters,
              bool isConstructor, Stmt* body) noexcept
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
   SourceRange location() const override { return location_; }
   ReturnType returnTy() const { return ReturnType{returnType_}; }

   utils::Generator<ast::AstNode const*> children() const override {
      co_yield returnType_;
      for(auto local : locals_) co_yield local;
      co_yield body_;
   }

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
