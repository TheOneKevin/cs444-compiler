#pragma once

#include <map>
#include <set>
#include <vector>

#include "ast/AST.h"
#include "diagnostics/Diagnostics.h"

namespace semantic {

class HierarchyChecker {
public:
   HierarchyChecker(diagnostics::DiagnosticEngine& diag): diag{diag} {}

   void Check(ast::LinkingUnit* lu) {
      lu_ = lu;
      checkInheritance();
   }

   bool isSubtype(ast::Type* sub, ast::Type* super);

   // @brief Check if a class is a subclass of another class
   bool isSuperClass(ast::ClassDecl* super, ast::ClassDecl* sub);

   // @brief Check if an interface is a subperinterface of another class/interface
   bool isSuperInterface(ast::InterfaceDecl* super, ast::Decl* sub);

private:
   diagnostics::DiagnosticEngine& diag;
   ast::LinkingUnit* lu_;
   std::pmr::map<ast::Decl*, std::pmr::set<ast::Decl*>> inheritanceMap_;
   void checkInheritance();

   // Check functions for method
   void checkClassConstructors(ast::ClassDecl* classDecl);
   void checkClassMethod(ast::ClassDecl* classDecl, std::pmr::vector<ast::MethodDecl*>& inheritedMethods);
   void checkInterfaceMethod(ast::InterfaceDecl* interfaceDecl, std::pmr::vector<ast::MethodDecl*>& inheritedMethods);

   // Check method inheritance
   void checkMethodInheritance();
   void checkMethodInheritanceHelper(
      ast::Decl* node,
      std::pmr::set<ast::Decl*>& visited);
};

} // namespace semantic
