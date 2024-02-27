#pragma once

#include "ast/AST.h"
#include "diagnostics/Diagnostics.h"

namespace semantic {

class TypeChecker {
public:
   TypeChecker(diagnostics::DiagnosticEngine& diag): diag{diag} {}

   void check(ast::LinkingUnit* lu) {
      lu_ = lu;
   }

private: 
   diagnostics::DiagnosticEngine& diag;
   ast::LinkingUnit* lu_;
};

} // namespace semantic