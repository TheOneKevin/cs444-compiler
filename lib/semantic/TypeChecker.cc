#include "TypeChecker.h"

namespace Semantic {

// Field initialization
// Method body
void TypeChecker::check(ast::LinkingUnit* lu) {
   lu_ = lu;
   for (auto cu : lu->compilationUnits) {
      
   }
}

void validateExpr(ast::Expr* expr) {
   pmr::stack<ast::ExprNode*> stack;
   for (exprNode : expr->rpn_ops.node()) {
      if (auto exprOp = dynamic_cast<ast::MemberAccess*>(exprNode)) {
         
      } else if (auto exprOp = dynamic_cast<ast::MethodInvocation*>(exprNode)){
         
      } else if (auto exprOp = dynamic_cast<ast::ClassInstanceCreation*>(exprNode)) {

      } else if (auto exprOp = dynamic_cast<ast::ArrayInstanceCreation*>(exprNode)) {

      } else if (auto exprOp = dynamic_cast<ast::ArrayAccess*>(exprNode)) {

      } else if (auto exprOp = dynamic_cast<ast::Cast*>(exprNode)) {

      } else if (auto exprOp = dynamic_cast<ast::BinaryOp*>(exprNode)) {
         validateBinaryOp(exprOp, stack);
      } else if (auto exprOp = dynamic_cast<ast::UnaryOp*>(exprNode)) {
         validateUnaryOp(exprOp, stack);
      } else {
         stack.push(exprNode);
      }

   }
}

void validateBinaryOp(ast::BinaryOp* binOp, pmr::stack<ast::ExprNode*>& stack) {
   auto right = stack.pop();
   auto left = stack.pop();
   
}

void validateUnaryOp(ast::UnaryOp* binOp, pmr::stack<ast::ExprNode*>& stack) {
   auto unaryExpression = stack.pop();
   // case 
   switch(binOp->getType()) {
      case ast::UnaryOp::Op::PLUS:

      case ast::UnaryOp::Op::MINUS:

      case ast::UnaryOp::Op::NOT:
         
   }
}

// Int, Byte, Char, Short (Type.h -> BuiltInType)

// Assignment returns null or void?

// another.object().method();
// expr->rpn_ops.nodes() -> [ , , , Method 

// [Another.Object]
// Method initialization

// Method formal parameters

} // namespace Semantic