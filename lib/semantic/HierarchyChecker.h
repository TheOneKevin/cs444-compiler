#pragma once

#include "ast/AST.h"
#include "diagnostics/Diagnostics.h"
#include <map>
#include <vector>
#include <set>

namespace semantic {

class HierarchyChecker {
public:
   HierarchyChecker(
      diagnostics::DiagnosticEngine& diag,
      ast::LinkingUnit* lu
   ) : diag{diag}, lu_{lu} {
      checkInheritance();
      checkMethodReplacement();
   }

private:
   diagnostics::DiagnosticEngine& diag;
   ast::LinkingUnit* lu_;
   std::pmr::map<ast::ClassDecl*, std::pmr::set<ast::ClassDecl*>> superClassMap_;
   std::pmr::map<ast::Decl*, std::pmr::set<ast::InterfaceDecl*>> superInterfaceMap_;
   void checkInheritance();

   // Check functinos for method 
   void checkClassConstructors(ast::ClassDecl* classDecl);
   void checkClassMethod(ast::ClassDecl* classDecl);
   void checkInterfaceMethod(ast::InterfaceDecl* interfaceDecl);
   void checkMethodReplacement();

   // Flattening map functions
   void flattenClassMap(std::pmr::map<ast::ClassDecl*, std::pmr::vector<ast::ClassDecl*>> &unflattenedMap);
   void flattenClassMapHelper(
      ast::ClassDecl* node,
      std::pmr::map<ast::ClassDecl*, std::pmr::vector<ast::ClassDecl*>> &unflattenedMap,
      std::pmr::set<ast::ClassDecl*> &visited,
      std::pmr::set<ast::ClassDecl*> &currentPath
   );
   void flattenInterfaceMap(std::pmr::map<ast::Decl*, std::pmr::vector<ast::InterfaceDecl*>> &unflattenedMap);
   void flattenInterfaceMapHelper(
      ast::Decl* node,
      std::pmr::map<ast::Decl*, std::pmr::vector<ast::InterfaceDecl*>> &unflattenedMap,
      std::pmr::set<ast::Decl*> &visited,
      std::pmr::set<ast::InterfaceDecl*> &currentPath
   );
};

} // namespace semantic
