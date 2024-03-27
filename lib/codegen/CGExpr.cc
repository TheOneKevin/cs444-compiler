#include "codegen/CGExpr.h"
#include <utility>

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
      assert(false && "Not implemented yet");
   } else if(auto thisNode = dyn_cast<ex::ThisNode>(node)) {
      assert(false && "Not implemented yet");
   } else if(auto methodName = dyn_cast<ex::MethodName>(node)) {
      assert(false && "Not implemented yet");
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
         assert(false && "Not implemented yet");
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
