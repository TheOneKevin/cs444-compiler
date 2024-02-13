#pragma once

#include <ranges>

#include "AstNode.h"

namespace ast {

class VarDecl;
class FieldDecl;
class MethodDecl;

struct ImportDeclaration {
   QualifiedIdentifier* qualifiedIdentifier;
   bool isOnDemand;
};

class CompilationUnit final : public DeclContext {
public:
   CompilationUnit(BumpAllocator& alloc,
                   QualifiedIdentifier* package,
                   array_ref<ImportDeclaration> imports,
                   DeclContext* body) noexcept
         : package_{package}, imports_{alloc}, body_{body} {
      // FIXME(kevin): Is there a better way to do this? vector + insert
      // seems like a very common pattern here.
      imports_.reserve(imports.size());
      imports_.insert(imports_.end(),
                      std::make_move_iterator(imports.begin()),
                      std::make_move_iterator(imports.end()));
   }
   auto body() const { return body_; }

   std::ostream& print(std::ostream& os, int indentation = 0) const override;
   int printDotNode(DotPrinter& dp) const override;

private:
   QualifiedIdentifier* package_;
   pmr_vector<ImportDeclaration> imports_;
   DeclContext* body_;
};

class ClassDecl : public DeclContext, public Decl {
public:
   ClassDecl(BumpAllocator& alloc,
             Modifiers modifiers,
             string_view name,
             QualifiedIdentifier* superClass,
             array_ref<QualifiedIdentifier*> interfaces,
             array_ref<Decl*> classBodyDecls);
   auto fields() const { return std::views::all(fields_); }
   auto methods() const { return std::views::all(methods_); }
   auto constructors() const { return std::views::all(constructors_); }
   auto interfaces() const { return std::views::all(interfaces_); }
   auto superClass() const { return superClass_; }
   auto modifiers() const { return modifiers_; }

   std::ostream& print(std::ostream& os, int indentation = 0) const override;
   int printDotNode(DotPrinter& dp) const override;

private:
   Modifiers modifiers_;
   QualifiedIdentifier* superClass_;
   pmr_vector<QualifiedIdentifier*> interfaces_;
   pmr_vector<FieldDecl*> fields_;
   pmr_vector<MethodDecl*> methods_;
   pmr_vector<MethodDecl*> constructors_;
};

class InterfaceDecl : public DeclContext, public Decl {
public:
   InterfaceDecl(BumpAllocator& alloc,
                 Modifiers modifiers,
                 string_view name,
                 array_ref<QualifiedIdentifier*> extends,
                 array_ref<Decl*> interfaceBodyDecls);
   auto extends() const { return std::views::all(extends_); }
   auto methods() const { return std::views::all(methods_); }
   auto modifiers() const { return modifiers_; }

   std::ostream& print(std::ostream& os, int indentation = 0) const override;
   int printDotNode(DotPrinter& dp) const override;

private:
   Modifiers modifiers_;
   pmr_vector<QualifiedIdentifier*> extends_;
   pmr_vector<MethodDecl*> methods_;
};

class MethodDecl : public DeclContext, public Decl {
public:
   MethodDecl(BumpAllocator& alloc,
              Modifiers modifiers,
              string_view name,
              Type* returnType,
              array_ref<VarDecl*> parameters,
              bool isConstructor,
              Stmt* body) noexcept
         : Decl{alloc, name},
           modifiers_{modifiers},
           returnType_{returnType},
           parameters_{alloc},
           isConstructor_{isConstructor},
           body_{body} {
      // FIXME(kevin): Is there a better way to do this? vector + insert
      // seems like a very common pattern here.
      parameters_.reserve(parameters.size());
      parameters_.insert(parameters_.end(),
                         std::make_move_iterator(parameters.begin()),
                         std::make_move_iterator(parameters.end()));
   }
   auto modifiers() const { return modifiers_; }
   bool isConstructor() const { return isConstructor_; }
   auto parameters() const { return std::views::all(parameters_); }

   std::ostream& print(std::ostream& os, int indentation = 0) const override;
   int printDotNode(DotPrinter& dp) const override;

private:
   Modifiers modifiers_;
   Type* returnType_;
   pmr_vector<VarDecl*> parameters_;
   bool isConstructor_;
   Stmt* body_;
};

} // namespace ast
