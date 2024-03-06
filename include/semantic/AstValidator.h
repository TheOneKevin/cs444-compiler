#pragma once

#include "ast/AST.h"
#include "ast/AstNode.h"
#include "ast/DeclContext.h"
#include "diagnostics/Diagnostics.h"
#include "semantic/ExprTypeResolver.h"
#include "utils/BumpAllocator.h"

namespace semantic {

class AstChecker {
public:
   AstChecker(BumpAllocator& alloc, diagnostics::DiagnosticEngine& diag,
              ExprTypeResolver& exprTypeResolver)
         : alloc{alloc}, diag{diag}, exprTypeResolver{exprTypeResolver} {}

   void ValidateLU(const ast::LinkingUnit& LU);

private:
   void validateCU(const ast::CompilationUnit& CU);
   void validateMethod(const ast::MethodDecl& method);
   void validateStmt(const ast::Stmt& stmt);
   void validateReturnStmt(const ast::ReturnStmt& stmt);
   void valdiateTypedDecl(const ast::TypedDecl& decl);

private:
   ast::Type const* getTypeFromExpr(ast::Expr const* expr) const;

private:
   ast::MethodDecl const* currentMethod;
   ast::CompilationUnit const* cu_;
   BumpAllocator& alloc;
   diagnostics::DiagnosticEngine& diag;
   ExprTypeResolver& exprTypeResolver;
};

} // namespace semantic
