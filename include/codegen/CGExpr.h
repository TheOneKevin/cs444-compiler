#pragma once

#include "ast/ExprEvaluator.h"
#include "codegen/CodeGen.h"
#include "tir/Constant.h"
#include "tir/Context.h"
#include "tir/TIR.h"

namespace codegen {

namespace details {

/**
 * @brief Wraps a value for expression evaluation. This distinguishes an
 * LValue from an RValue, wrapping the tir::Value* mapped from an ExprValue.
 * The criteria for an L/R value is as follows:
 *    - Constants are wrapped R values always
 *    - Var and field decls are wrapped as L values always
 *    - Expressions are wrapped as R values always
 *    - Function decls are neither L nor R values
 * The tir::Value can then be unwrapped using a conversion function.
 */
class ValueWrapper {
public:
   enum class Kind { StaticFn, MemberFn, L, R };
   // Wrap a value as an RValue
   explicit ValueWrapper(tir::Value* value, Kind kind = Kind::R)
         : kind_{kind}, type_{value->type()}, value_{value} {
      assert(kind != Kind::L && !type_->isPointerType());
   }
   // Wrap a (pointer) value as an LValue
   explicit ValueWrapper(tir::Type* type, tir::Value* value)
         : kind_{Kind::L}, type_{type}, value_{value} {
      assert(value_->type()->isPointerType());
   }

   tir::Value* asRValue(tir::IRBuilder&) const;
   tir::Value* asLValue() const;
   tir::Value* asValue() const { return value_; }
   bool isStaticFn() const { return kind_ == Kind::StaticFn; }
   bool isMemberFn() const { return kind_ == Kind::MemberFn; }

private:
   Kind kind_;
   tir::Type* type_;
   tir::Value* value_;
};

} // namespace details

/**
 * @brief The code generator expression evaluator. This class is responsible
 * for evaluating expressions and returning the corresponding tir::Value*.
 */
class CGExprEvaluator final : public ast::ExprEvaluator<details::ValueWrapper> {
public:
   using T = details::ValueWrapper;
   explicit CGExprEvaluator(CodeGenerator& cg) : cg{cg} {}

private:
   T mapValue(ast::exprnode::ExprValue& node) const override;
   T evalBinaryOp(ast::exprnode::BinaryOp& op, T lhs, T rhs) const override;
   T evalUnaryOp(ast::exprnode::UnaryOp& op, T rhs) const override;
   T evalMemberAccess(ast::exprnode::MemberAccess& op, T lhs,
                      T field) const override;
   T evalMethodCall(ast::exprnode::MethodInvocation& op, T method,
                    const op_array& args) const override;
   T evalNewObject(ast::exprnode::ClassInstanceCreation& op, T object,
                   const op_array& args) const override;
   T evalNewArray(ast::exprnode::ArrayInstanceCreation& op, T type,
                  T size) const override;
   T evalArrayAccess(ast::exprnode::ArrayAccess& op, T array,
                     T index) const override;
   T evalCast(ast::exprnode::Cast& op, T type, T value) const override;
   bool validate(T const& v) const override {
      return v.asValue() != nullptr;
   }

private:
   CodeGenerator& cg;
   tir::Context& ctx{cg.ctx};
   tir::CompilationUnit& cu{cg.cu};
   tir::Function& curFn{*cg.curFn};
};

} // namespace codegen
