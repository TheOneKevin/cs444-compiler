#include "codegen/CGExpr.h"

#include <utility>

#include "ast/Decl.h"
#include "codegen/CodeGen.h"
#include "tir/Constant.h"
#include "tir/IRBuilder.h"
#include "tir/Instructions.h"
#include "tir/Type.h"
#include "utils/Utils.h"

using namespace tir;
namespace ex = ast::exprnode;
using T = codegen::CGExprEvaluator::T;

/* ===--------------------------------------------------------------------=== */
// Conversion functions in T::
/* ===--------------------------------------------------------------------=== */

tir::Value* T::asRValue(tir::IRBuilder& builder) const {
   assert(kind_ == Kind::L || kind_ == Kind::R);
   if(kind_ == Kind::L) {
      return builder.createLoadInstr(type_, value_);
   } else {
      return value_;
   }
}

tir::Value* T::asLValue() const {
   assert(kind_ == Kind::L);
   return value_;
}

/* ===--------------------------------------------------------------------=== */
// Emit specific expressions
/* ===--------------------------------------------------------------------=== */

namespace codegen {

T CGExprEvaluator::mapValue(ex::ExprValue& node) const {
   if(auto memberName = dyn_cast<ex::MemberName>(node)) {
      auto ty = cg.emitType(cast<ast::TypedDecl>(memberName->decl())->type());
      // 1. If it's a field decl, handle the static and non-static cases
      if(auto* fieldDecl = dyn_cast<ast::FieldDecl>(memberName->decl())) {
         // 1. If it's static, then grab the GV
         if(fieldDecl->modifiers().isStatic()) {
            return T{ty, cu.findGlobalVariable(fieldDecl->getCanonicalName())};
         }
         // 2. Otherwise... it's complicated
         else {
            assert(false && "Not implemented yet");
         }
      }
      // 2. If it's a local (var) decl, grab the alloca inst
      else {
         auto* localDecl = cast<ast::VarDecl>(memberName->decl());
         return T{ty, cg.valueMap[localDecl]};
      }
   } else if(auto thisNode = dyn_cast<ex::ThisNode>(node)) {
      // "this" will be the first argument of the function
      return T{curFn.args().front()};
   } else if(auto methodName = dyn_cast<ex::MethodName>(node)) {
      auto* methodDecl = cast<ast::MethodDecl>(methodName->decl());
      // 1. If it's static, then grab the GV
      if(methodDecl->modifiers().isStatic()) {
         return T{cu.findFunction(methodDecl->getCanonicalName()),
                  T::Kind::StaticFn};
      }
      // 2. Otherwise... it's complicated
      else {
         assert(false && "Not implemented yet");
      }
   } else if(auto literal = dyn_cast<ex::LiteralNode>(node)) {
      if(literal->builtinType()->isNumeric()) {
         return T{Constant::CreateInt32(ctx, literal->getAsInt())};
      } else if(literal->builtinType()->isBoolean()) {
         return T{Constant::CreateBool(ctx, literal->getAsInt())};
      } else if(literal->builtinType()->isString()) {
         // String type
         assert(false && "Not implemented yet");
      } else {
         // Null type
         return T{Constant::CreateNullPointer(ctx)};
      }
   }
   std::unreachable();
}

static CmpInst::Predicate getPredicate(ex::BinaryOp::OpType op) {
   using OpType = ex::BinaryOp::OpType;
   switch(op) {
      case OpType::GreaterThan:
         return CmpInst::Predicate::GT;
      case OpType::GreaterThanOrEqual:
         return CmpInst::Predicate::GE;
      case OpType::LessThan:
         return CmpInst::Predicate::LT;
      case OpType::LessThanOrEqual:
         return CmpInst::Predicate::LE;
      case OpType::Equal:
         return CmpInst::Predicate::EQ;
      case OpType::NotEqual:
         return CmpInst::Predicate::NE;
      default:
         assert(false);
   }
   std::unreachable();
}

static Instruction::BinOp getBinOp(ex::BinaryOp::OpType op) {
   using OpType = ex::BinaryOp::OpType;
   switch(op) {
      case OpType::BitwiseAnd:
         return Instruction::BinOp::And;
      case OpType::BitwiseOr:
         return Instruction::BinOp::Or;
      case OpType::BitwiseXor:
         return Instruction::BinOp::Xor;
      case OpType::Add:
         return Instruction::BinOp::Add;
      case OpType::Subtract:
         return Instruction::BinOp::Sub;
      case OpType::Multiply:
         return Instruction::BinOp::Mul;
      case OpType::Divide:
         return Instruction::BinOp::Div;
      case OpType::Modulo:
         return Instruction::BinOp::Rem;
      default:
         assert(false);
   }
   std::unreachable();
}

T CGExprEvaluator::evalBinaryOp(ex::BinaryOp& op, T lhs, T rhs) const {
   using OpType = ex::BinaryOp::OpType;
   switch(op.opType()) {
      // Assignment expression //
      case OpType::Assignment:
         cg.builder.createStoreInstr(rhs.asRValue(cg.builder), lhs.asLValue());
         return lhs;

      // Comparison expressions //
      case OpType::GreaterThan:
      case OpType::GreaterThanOrEqual:
      case OpType::LessThan:
      case OpType::LessThanOrEqual:
      case OpType::Equal:
      case OpType::NotEqual:
         return T{cg.builder.createCmpInstr(getPredicate(op.opType()),
                                            lhs.asRValue(cg.builder),
                                            rhs.asRValue(cg.builder))};

      // Short circuit expressions //
      case OpType::And: {
         /*
            curBB:
               %v0 = i1 eval(lhs)
               store i1 %v0, %tmp
               br i1 %v0, bb1, bb2
            bb1:
               %v1 = i1 eval(rhs)
               store i1 %v1, %tmp
               br bb2
            bb2:
               %tmp as lvalue
         */
         auto tmp = cg.curFn->createAlloca(Type::getInt1Ty(ctx));
         auto bb1 = cg.builder.createBasicBlock(&curFn);
         auto bb2 = cg.builder.createBasicBlock(&curFn);
         auto v0 = lhs.asRValue(cg.builder);
         cg.builder.createStoreInstr(v0, tmp);
         cg.builder.createBranchInstr(v0, bb1, bb2);
         cg.builder.setInsertPoint(bb1);
         auto v1 = rhs.asRValue(cg.builder);
         cg.builder.createStoreInstr(v1, tmp);
         cg.builder.createBranchInstr(bb2);
         cg.builder.setInsertPoint(bb2);
         return T{Type::getInt1Ty(ctx), tmp};
      }
      case OpType::Or: {
         /*
            curBB:
               v0 = i1 eval(lhs)
               store i1 %v0, %tmp
               br i1 %v0, bb2, bb1
            bb1:
               v1 = i1 eval(rhs)
               store i1 %v1, %tmp
               br bb2
            bb2:
               %tmp as lvalue
         */
         auto tmp = cg.curFn->createAlloca(Type::getInt1Ty(ctx));
         auto bb1 = cg.builder.createBasicBlock(&curFn);
         auto bb2 = cg.builder.createBasicBlock(&curFn);
         auto v0 = lhs.asRValue(cg.builder);
         cg.builder.createStoreInstr(v0, tmp);
         cg.builder.createBranchInstr(v0, bb2, bb1);
         cg.builder.setInsertPoint(bb1);
         auto v1 = rhs.asRValue(cg.builder);
         cg.builder.createStoreInstr(v1, tmp);
         cg.builder.createBranchInstr(bb2);
         cg.builder.setInsertPoint(bb2);
         return T{Type::getInt1Ty(ctx), tmp};
      }

      // Arithmetic expressions //
      case OpType::BitwiseAnd:
      case OpType::BitwiseOr:
      case OpType::BitwiseXor:
      case OpType::Add:
      case OpType::Subtract:
      case OpType::Multiply:
      case OpType::Divide:
      case OpType::Modulo:
         return T{cg.builder.createBinaryInstr(getBinOp(op.opType()),
                                               lhs.asRValue(cg.builder),
                                               rhs.asRValue(cg.builder))};

      // Instance of expression //
      case OpType::InstanceOf:
         assert(false && "Not implemented yet");
      default:
         break;
   }
   std::unreachable();
}

T CGExprEvaluator::evalUnaryOp(ex::UnaryOp& op, T rhs) const {
   using OpType = ex::UnaryOp::OpType;
   auto value = rhs.asRValue(cg.builder);
   auto ty = value->type();
   switch(op.opType()) {
      case OpType::Not:
      case OpType::BitwiseNot: {
         auto allOnes = ConstantInt::AllOnes(ctx, ty);
         return T{cg.builder.createBinaryInstr(
               Instruction::BinOp::Xor, value, allOnes)};
      }
      case OpType::Plus:
         return rhs;
      case OpType::Minus:
         return T{cg.builder.createBinaryInstr(
               Instruction::BinOp::Sub, ConstantInt::Zero(ctx, ty), value)};
      default:
         break;
   }
   std::unreachable();
}

T CGExprEvaluator::evalMemberAccess(ex::MemberAccess& op, T lhs, T field) const {
   // Function body for evalMemberAccess
   // TODO: Implement the logic for evaluating member access
   assert(false);
}

T CGExprEvaluator::evalMethodCall(ex::MethodInvocation& op, T method,
                                  const op_array& args) const {
   (void)op;
   // Check if the method is static or not
   if(method.isStaticFn()) {
      std::vector<Value*> argValues;
      for(auto& arg : args) {
         argValues.push_back(arg.asRValue(cg.builder));
      }
      return T{cg.builder.createCallInstr(method.asValue(), argValues)};
   }
   assert(method.isMemberFn());
   assert(false);
}

T CGExprEvaluator::evalNewObject(ex::ClassInstanceCreation& op, T object,
                                 const op_array& args) const {
   // Function body for evalNewObject
   // TODO: Implement the logic for evaluating new object creation
   assert(false);
}

T CGExprEvaluator::evalNewArray(ex::ArrayInstanceCreation& op, T type,
                                T size) const {
   // Function body for evalNewArray
   // TODO: Implement the logic for evaluating new array creation
   assert(false);
}

T CGExprEvaluator::evalArrayAccess(ex::ArrayAccess& op, T array, T index) const {
   // Function body for evalArrayAccess
   // TODO: Implement the logic for evaluating array access
   assert(false);
}

T CGExprEvaluator::evalCast(ex::Cast& op, T type, T value) const {
   // Function body for evalCast
   // TODO: Implement the logic for evaluating type casting
   assert(false);
}

} // namespace codegen

/* ===--------------------------------------------------------------------=== */
// CodeGenerator emit router
/* ===--------------------------------------------------------------------=== */

namespace codegen {

Value* CodeGenerator::emitExpr(ast::Expr const* expr) {
   CGExprEvaluator eval{*this};
   T result = eval.EvaluateList(expr->list());
   return result.asRValue(builder);
}

} // namespace codegen
