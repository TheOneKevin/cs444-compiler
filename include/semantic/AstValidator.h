#pragma once

#include "ast/AST.h"
#include "ast/DeclContext.h"
#include "diagnostics/Diagnostics.h"
#include "utils/BumpAllocator.h"
#include "semantic/ExprTypeResolver.h"

namespace semantic {

class AstChecker {
public:
   AstChecker(BumpAllocator& alloc, diagnostics::DiagnosticEngine& diag, ExprTypeResolver& exprTypeResolver)
         : alloc{alloc}, diag{diag}, exprTypeResolver{exprTypeResolver} {}

   void ValidateLU(const ast::LinkingUnit& LU);
   void ValidateCU(const ast::CompilationUnit& CU);
   void ValidateMethod(const ast::MethodDecl& method);
   bool RecursiveFindReturnStmt(const ast::AstNode& stmt, const ast::Type* returnValueType, const ast::Type* methodReturnType, const bool isVoid);

private:
   ast::CompilationUnit const* cu_;
   BumpAllocator& alloc;
   diagnostics::DiagnosticEngine& diag;
   ExprTypeResolver& exprTypeResolver;
};

} // namespace semantic
