#pragma once

#include <map>
#include <set>
#include <vector>

#include "ast/AST.h"
#include "ast/AstNode.h"
#include "diagnostics/Diagnostics.h"

namespace semantic {

class HierarchyChecker {
public:
   HierarchyChecker(diagnostics::DiagnosticEngine& diag) : diag{diag} {}

   void Check(ast::LinkingUnit const* lu) {
      lu_ = lu;
      checkInheritance();
   }

   bool isSubtype(ast::Type const* sub, ast::Type const* super);

   // @brief Check if a class is a subclass of another class
   bool isSuperClass(ast::ClassDecl const* super, ast::ClassDecl const* sub);

   // @brief Check if an interface is a subperinterface of another class/interface
   bool isSuperInterface(ast::InterfaceDecl const* super, ast::Decl const* sub);

   bool isInheritedSet(ast::Decl const* decl) {
      return methodInheritanceMap_.find(decl) != methodInheritanceMap_.end();
   }

   auto& getInheritedMethods(ast::Decl const* decl) {
      assert(isInheritedSet(decl));
      std::vector<int> x;
      return methodInheritanceMap_.at(decl);
   }

   auto& getInheritedMembers(ast::Decl const* decl) {
      // assert(memberInheritancesMap_.find(decl) != memberInheritancesMap_.end());
      return memberInheritancesMap_[decl];
   }

   void setInheritedMethods(
         ast::Decl const* decl,
         std::pmr::vector<ast::MethodDecl const*>& inheritedMethods) {
      assert(inheritanceMap_.find(decl) != inheritanceMap_.end());
      methodInheritanceMap_.insert({decl, inheritedMethods});
   }

private:
   diagnostics::DiagnosticEngine& diag;
   ast::LinkingUnit const* lu_;
   std::pmr::map<ast::Decl const*, std::pmr::set<ast::Decl const*>>
         inheritanceMap_;
   std::pmr::map<ast::Decl const*, std::pmr::vector<ast::MethodDecl const*>>
         methodInheritanceMap_;
   std::pmr::map<ast::Decl const*, std::pmr::set<ast::TypedDecl const*>>
         memberInheritancesMap_;
   void checkInheritance();

   // Check functions for method
   void checkClassConstructors(ast::ClassDecl const* classDecl);
   void checkClassMethod(
         ast::ClassDecl const* classDecl,
         std::pmr::vector<ast::MethodDecl const*>& inheritedMethods);
   void checkInterfaceMethod(
         ast::InterfaceDecl const* interfaceDecl,
         std::pmr::vector<ast::MethodDecl const*>& inheritedMethods);

   // Check method inheritance
   void checkMethodInheritance();
   void checkMethodInheritanceHelper(ast::Decl const* node,
                                     std::pmr::set<ast::Decl const*>& visited);

   void setInheritedMembersHelper(ast::Decl const* node, ast::Decl const* parent);
};

} // namespace semantic
