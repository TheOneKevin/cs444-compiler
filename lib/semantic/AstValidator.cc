#include "semantic/AstValidator.h"

#include <cassert>
#include <utility>

#include "ast/Decl.h"
#include "ast/ExprNode.h"
#include "ast/Stmt.h"
#include "semantic/ExprTypeResolver.h"
#include "utils/Utils.h"

namespace semantic {

void AstChecker::ValidateLU(const ast::LinkingUnit& LU) {
   cu_ = nullptr;
   for(auto* cu : LU.compliationUnits()) {
      validateCU(*cu);
   }
}

void AstChecker::validateCU(const ast::CompilationUnit& CU) {
   cu_ = &CU;
   if(!CU.bodyAsDecl()) return;
   for(auto* child : CU.bodyAsDecl()->children()) {
      if(auto* method = dyn_cast_or_null<ast::MethodDecl>(child)) {
         validateMethod(*method);
      } else if(auto* field = dyn_cast_or_null<ast::FieldDecl>(child)) {
         valdiateTypedDecl(*field);
      }
   }
}

void AstChecker::validateMethod(const ast::MethodDecl& method) {
   currentMethod = &method;
   if(method.body()) validateStmt(*method.body());
   currentMethod = nullptr;
}

void AstChecker::validateStmt(const ast::Stmt& stmt) {
   if(auto ret = dyn_cast<ast::ReturnStmt>(stmt)) {
      validateReturnStmt(*ret);
   } else if(auto decl = dyn_cast<ast::DeclStmt>(stmt)) {
      valdiateTypedDecl(*decl->decl());
   } else if(auto ifstmt = dyn_cast<ast::IfStmt>(stmt)) {
      assert(ifstmt->condition());
      auto* condTy = getTypeFromExpr(ifstmt->condition());
      if(!condTy || !condTy->isBoolean()) {
         diag.ReportError(ifstmt->condition()->location())
               << "if condition expression must be yield a boolean";
      }
   } else if(auto forstmt = dyn_cast<ast::ForStmt>(stmt)) {
      if (forstmt->condition()) {
         auto* condTy = getTypeFromExpr(forstmt->condition());
         if(!condTy || !condTy->isBoolean()) {
            diag.ReportError(forstmt->condition()->location())
                  << "for condition expression must be yield a boolean";
         }
      }
   } else if(auto whilestmt = dyn_cast<ast::WhileStmt>(stmt)) {
      assert(whilestmt->condition());
      auto* condTy = getTypeFromExpr(whilestmt->condition());
      if(!condTy || !condTy->isBoolean()) {
         diag.ReportError(whilestmt->condition()->location())
               << "while condition expression must be yield a boolean";
      }
   }
   for(auto* child : stmt.children()) {
      if(auto* stmt = dyn_cast_or_null<ast::Stmt>(child)) {
         validateStmt(*stmt);
      }
   }
}

ast::Type const* AstChecker::getTypeFromExpr(ast::Expr const* expr) const {
   assert(expr);
   auto lastExpr = expr->list().tail();
   assert(lastExpr);
   if(auto op = dyn_cast<ast::exprnode::ExprOp>(lastExpr)) {
      return op->resultType();
   } else if(auto value = dyn_cast<ast::exprnode::ExprValue>(lastExpr)) {
      assert(value->type() && "Expr value type cannot be null");
      return value->type();
   }
   std::unreachable();
}

void AstChecker::valdiateTypedDecl(const ast::TypedDecl& decl) {
   if(!decl.hasInit()) return;
   auto* declTy = decl.type();
   auto* exprTy = getTypeFromExpr(decl.init());
   if(!exprTy) {
      diag.ReportError(decl.init()->location())
            << "initializer type cannot be void";
      return;
   }
   if(!exprTypeResolver.isAssignableTo(declTy, exprTy)) {
      diag.ReportError(decl.location())
            << "initializer type must be assignable to declared type";
   }
}

void AstChecker::validateReturnStmt(const ast::ReturnStmt& ret) {
   assert(currentMethod);
   auto methodRetTy = currentMethod->returnTy().type;
   if(!ret.expr()) {
      if(methodRetTy) {
         diag.ReportError(ret.location())
               << "return must be non-void" << ret.location() << "return is here"
               << currentMethod->location() << "method declared here";
      }
      return;
   }

   ast::Type const* retTy = getTypeFromExpr(ret.expr());
   if(!retTy) {
      diag.ReportError(ret.expr()->location())
            << "return expression cannot be void, regardless of method return "
               "type";
      return;
   }

   if(!methodRetTy) {
      if(retTy) {
         diag.ReportError(ret.location())
               << "return must be void" << ret.location() << "return is here"
               << currentMethod->location() << "method declared here";
      }
      return;
   }

   if(!exprTypeResolver.isAssignableTo(methodRetTy, retTy)) {
      diag.ReportError(ret.location())
            << "return type must be assignable to method return type"
            << ret.location() << "return is type " << retTy->toString()
            << currentMethod->location() << "method declared here";
   }
}

} // namespace semantic