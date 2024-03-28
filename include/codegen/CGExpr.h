#pragma once

#include "ast/AstNode.h"
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
   struct TirWrapped {
      ast::Type const* astType;
      tir::Type* type;
      tir::Value* value;
   };

public:
   enum class Kind { StaticFn, MemberFn, AstType, AstDecl, L, R };

   /**
    * @brief Wrap an AST Type node. This can not be an IR value.
    *
    * @param aty The AST type node to wrap.
    */
   explicit ValueWrapper(ast::Type const* aty)
         : kind_{Kind::AstType}, data_{aty} {}

   /**
    * @brief Wrap an AST FieldDecl node. This can not be an IR value.
    *
    * @param decl The AST field declaration node to wrap.
    */
   explicit ValueWrapper(ast::Decl const* decl)
         : kind_{Kind::AstDecl}, data_{decl} {}

private:
   // Private constructor for wrapping IR values
   explicit ValueWrapper(Kind kind, TirWrapped wrappedIr)
         : kind_{kind}, data_{wrappedIr} {}

public:
   /**
    * @brief Create an L-value wrapper for a pointer value.
    *
    * @param aty The AST type of the reference.
    * @param elemTy The dereferenced IR type.
    * @param value The pointer value to wrap.
    * @return ValueWrapper
    */
   static ValueWrapper L(ast::Type const* aty, tir::Type* elemTy,
                         tir::Value* value) {
      return ValueWrapper{Kind::L, {aty, elemTy, value}};
   }

   /**
    * @brief Create an R-value wrapper for a non-pointer value.
    *
    * @param aty The AST type of the value.
    * @param value The IR value to wrap.
    * @return ValueWrapper The wrapped value.
    */
   static ValueWrapper R(ast::Type const* aty, tir::Value* value) {
      assert(!value->type()->isPointerType());
      return ValueWrapper{Kind::R, {aty, value->type(), value}};
   }

   /**
    * @brief Wrap a static function value or a member function value.
    *
    * @param value The IR function value to wrap.
    * @return ValueWrapper The wrapped value.
    */
   static ValueWrapper Fn(Kind kind, tir::Value* value) {
      assert(kind == Kind::StaticFn || kind == Kind::MemberFn);
      return ValueWrapper{kind, {nullptr, nullptr, value}};
   }

   // Gets the wrapped IR value as an R-value
   tir::Value* asRValue(tir::IRBuilder&) const;
   // Gets the wrapped IR L-value
   tir::Value* asLValue() const;
   // Gets the wrapped IR value as a method/function
   tir::Value* asFn() const;
   // Gets the AST type of the IR value or the wrapped AST type
   ast::Type const* astType() const;
   // Gets the IR element type of the L-value or the type of the R-value
   tir::Type* irType() const;
   // Gets the kind of the wrapped value
   Kind kind() const { return kind_; }
   // Validates the wrapped value
   bool validate(CodeGenerator& cg) const;
   // Gets the wrapped AST declaration
   ast::Decl const* asDecl() const;

private:
   Kind kind_;
   std::variant<TirWrapped, ast::Type const*, ast::Decl const*> data_;
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
   bool validate(T const& v) const override { return v.validate(cg); }
   T castIntegerType(ast::Type const*, tir::Type*, T value) const;

private:
   CodeGenerator& cg;
   tir::Context& ctx{cg.ctx};
   tir::CompilationUnit& cu{cg.cu};
   tir::Function& curFn{*cg.curFn};
};

} // namespace codegen
