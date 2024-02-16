#pragma once

#include "ast/AST.h"
#include "diagnostics/Diagnostics.h"
#include <map>
#include <vector>

namespace semantic {

class HierarchyChecker {
public:
   HierarchyChecker(
      diagnostics::DiagnosticEngine& diag,
      ast::LinkingUnit* lu
   ) : diag{diag}, lu_{lu} {
      checkInheritance();
   }

private:
   diagnostics::DiagnosticEngine& diag;
   ast::LinkingUnit* lu_;
   std::pmr::map<ast::Decl*, std::pmr::vector<ast::Decl*>> hierarchyMap;
   void checkInheritance();
   void checkClassConstructors(ast::ClassDecl* classDecl);
   void checkClassMethod(ast::ClassDecl* classDecl);
   void checkInterfaceMethod(ast::InterfaceDecl* interfaceDecl);
};

} // namespace semantic
