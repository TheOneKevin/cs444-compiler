#include "codegen/CGExpr.h"

#include <utility>

#include "ast/Decl.h"
#include "codegen/CodeGen.h"
#include "utils/Utils.h"

using namespace tir;
namespace ex = ast::exprnode;

/* ===--------------------------------------------------------------------=== */
// Emit specific expressions
/* ===--------------------------------------------------------------------=== */

namespace codegen {

tir::Value* CGExprEvaluator::mapValue(ast::exprnode::ExprValue& node) const {
   if(auto memberName = dyn_cast<ex::MemberName>(node)) {
      // 1. If it's a field decl, handle the static and non-static cases
      if(auto* fieldDecl = dyn_cast<ast::FieldDecl>(memberName->decl())) {
         // 1. If it's static, then grab the GV
         if(fieldDecl->modifiers().isStatic()) {
            return cu.findGlobalVariable(fieldDecl->getCanonicalName());
         }
         // 2. Otherwise... it's complicated
         else {
            assert(false && "Not implemented yet");
         }
      }
      // 2. If it's a local (var) decl, grab the alloca inst
      else {
         auto* localDecl = cast<ast::VarDecl>(memberName->decl());
         return cg.valueMap[localDecl];
      }
   } else if(auto thisNode = dyn_cast<ex::ThisNode>(node)) {
      // "this" will be the first argument of the function
      return curFn.args().front();
   } else if(auto methodName = dyn_cast<ex::MethodName>(node)) {
      auto* methodDecl = cast<ast::MethodDecl>(methodName->decl());
      // 1. If it's static, then grab the GV
      if(methodDecl->modifiers().isStatic()) {
         return cu.findFunction(methodDecl->getCanonicalName());
      }
      // 2. Otherwise... it's complicated
      else {
         assert(false && "Not implemented yet");
      }
   } else if(auto literal = dyn_cast<ex::LiteralNode>(node)) {
      if(literal->builtinType()->isNumeric()) {
         return Constant::CreateInt32(ctx, literal->getAsInt());
      } else if(literal->builtinType()->isBoolean()) {
         return Constant::CreateBool(ctx, literal->getAsInt());
      } else if(literal->builtinType()->isString()) {
         // String type
         assert(false && "Not implemented yet");
      } else {
         // Null type
         return Constant::CreateNullPointer(ctx);
      }
   }
   std::unreachable();
}

tir::Value* CGExprEvaluator::evalBinaryOp(ast::exprnode::BinaryOp& op,
                                          tir::Value* lhs, tir::Value* rhs) const {
   // Function body for evalBinaryOp
   // TODO: Implement the logic for evaluating binary operations
   return nullptr; // Placeholder return statement
}

tir::Value* CGExprEvaluator::evalUnaryOp(ast::exprnode::UnaryOp& op,
                                         tir::Value* rhs) const {
   // Function body for evalUnaryOp
   // TODO: Implement the logic for evaluating unary operations
   return nullptr; // Placeholder return statement
}

tir::Value* CGExprEvaluator::evalMemberAccess(ast::exprnode::MemberAccess& op,
                                              tir::Value* lhs,
                                              tir::Value* field) const {
   // Function body for evalMemberAccess
   // TODO: Implement the logic for evaluating member access
   return nullptr; // Placeholder return statement
}

tir::Value* CGExprEvaluator::evalMethodCall(ast::exprnode::MethodInvocation& op,
                                            tir::Value* method,
                                            const op_array& args) const {
   // Function body for evalMethodCall
   // TODO: Implement the logic for evaluating method calls
   return nullptr; // Placeholder return statement
}

tir::Value* CGExprEvaluator::evalNewObject(
      ast::exprnode::ClassInstanceCreation& op, tir::Value* object,
      const op_array& args) const {
   // Function body for evalNewObject
   // TODO: Implement the logic for evaluating new object creation
   return nullptr; // Placeholder return statement
}

tir::Value* CGExprEvaluator::evalNewArray(ast::exprnode::ArrayInstanceCreation& op,
                                          tir::Value* type,
                                          tir::Value* size) const {
   // Function body for evalNewArray
   // TODO: Implement the logic for evaluating new array creation
   return nullptr; // Placeholder return statement
}

tir::Value* CGExprEvaluator::evalArrayAccess(ast::exprnode::ArrayAccess& op,
                                             tir::Value* array,
                                             tir::Value* index) const {
   // Function body for evalArrayAccess
   // TODO: Implement the logic for evaluating array access
   return nullptr; // Placeholder return statement
}

tir::Value* CGExprEvaluator::evalCast(ast::exprnode::Cast& op, tir::Value* type,
                                      tir::Value* value) const {
   // Function body for evalCast
   // TODO: Implement the logic for evaluating type casting
   return nullptr; // Placeholder return statement
}

} // namespace codegen

/* ===--------------------------------------------------------------------=== */
// CodeGenerator emit router
/* ===--------------------------------------------------------------------=== */

namespace codegen {

Value* CodeGenerator::emitExpr(ast::Expr const* expr) {
   assert(false && "Not implemented yet");
}

} // namespace codegen
