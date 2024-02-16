#pragma once

#include "ast/AST.h"
#include "diagnostics/Diagnostics.h"

namespace semantic {

class HierarchyChecker {
public:
   HierarchyChecker(
      diagnostics::DiagnosticEngine& diag,
      ast::LinkingUnit* lu
   ) : lu_{lu}, diag{diag} {
      checkInheritance();
   }

private:
   diagnostics::DiagnosticEngine& diag;
   ast::LinkingUnit* lu_;
   void checkInheritance();
};

} // namespace semantic
