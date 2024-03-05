#pragma once

#include "ast/AST.h"
#include "ast/DeclContext.h"
#include "diagnostics/Diagnostics.h"
#include "utils/BumpAllocator.h"

namespace semantic {

class AstChecker {
public:
   AstChecker(BumpAllocator& alloc, diagnostics::DiagnosticEngine& diag)
         : alloc{alloc}, diag{diag} {}

public:
   void ValidateLU(const ast::LinkingUnit& LU);
   void ValidateCU(const ast::CompilationUnit& CU);
   void ValidateMethod(const ast::MethodDecl& method);

private:
   ast::CompilationUnit const* cu_;
   BumpAllocator& alloc;
   diagnostics::DiagnosticEngine& diag;
};

} // namespace semantic
