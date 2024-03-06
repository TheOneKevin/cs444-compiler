#include "semantic/AstValidator.h"
#include "ast/ExprNode.h"
#include "semantic/ExprTypeResolver.h"
#include "utils/Utils.h"

namespace semantic {

void AstChecker::ValidateLU(const ast::LinkingUnit& LU) {
   cu_ = nullptr;
   for(auto* cu : LU.compliationUnits()) {
      ValidateCU(*cu);
   }
}

void AstChecker::ValidateCU(const ast::CompilationUnit& CU) {
   cu_ = &CU;

   if (!CU.bodyAsDecl()) return;

   for(auto* children : CU.bodyAsDecl()->children()) {
      if(auto* method = dyn_cast_or_null<ast::MethodDecl>(children)) {
         ValidateMethod(*method);
      }
   }
}

void AstChecker::ValidateMethod(const ast::MethodDecl& method) {
   auto methodReturnType = method.returnTy().type;
   const ast::Type* returnValueType;

   bool isVoid = methodReturnType == nullptr;

   bool foundReturn = true;

   if(auto* block = method.body()) {
      for(auto* stmt : block->children()) {
         foundReturn &= RecursiveFindReturnStmt(*stmt, returnValueType, methodReturnType, isVoid);
      }
   }

   if (!foundReturn) {
      diag.ReportError(method.location()) << "Invalid return statement type";
   }
}

bool AstChecker::RecursiveFindReturnStmt(const ast::AstNode& stmt, const ast::Type* returnValueType, const ast::Type* methodReturnType, const bool isVoid) {
   if (auto* ret = dyn_cast<ast::ReturnStmt>(stmt)) {
      if (isVoid) {
         return ret->expr() == nullptr;
      }

      auto lastExpr = ret->expr()->list().tail();
      
      if(auto op = dyn_cast<ast::exprnode::ExprOp>(lastExpr)) {
         assert(op->resultType() && "Expr op result type cannot be null");
         return exprTypeResolver.isAssignableTo(methodReturnType, op->resultType());
      } else if (auto value = dyn_cast<ast::exprnode::ExprValue>(lastExpr)) {
         assert(value->type() && "Expr value type cannot be null");
         return exprTypeResolver.isAssignableTo(methodReturnType, value->type());
      } else {
         assert(false);
      }
   }

   bool foundReturn = true;

   for(auto* stmt : stmt.children()) {
      if (!stmt) continue;
      foundReturn &= RecursiveFindReturnStmt(*stmt, returnValueType, methodReturnType, isVoid);
   }

   return foundReturn;
}

}