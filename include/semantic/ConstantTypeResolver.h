#pragma once

#include <memory_resource>

#include "ast/AstNode.h"
#include "ast/Expr.h"
#include "ast/ExprEvaluator.h"
#include "ast/ExprNode.h"
#include "diagnostics/Diagnostics.h"
#include "diagnostics/Location.h"
#include "semantic/HierarchyChecker.h"
#include "semantic/NameResolver.h"
#include "semantic/Semantic.h"
#include "utils/BumpAllocator.h"

namespace semantic {

struct ConstantReturnType {
   enum type { 
      BOOL, 
      INT, 
      UNKNOWN 
   } constantType;

   int value;

   ConstantReturnType(type constantType, int value) 
      : constantType{constantType}, value{value} {}
};

/**
 * @brief Will resolve a constant boolean expression if possible, or return a
 * nullptr if the expression is not a constant expression.
 */
class ConstantTypeResolver final : private ast::ExprEvaluator<ConstantReturnType const*> {
public:
   ConstantTypeResolver(BumpAllocator& alloc) : alloc{alloc} {}

   ConstantReturnType const* EvalList(ast::ExprNodeList& list) {
      return ast::ExprEvaluator<ConstantReturnType const*>::EvaluateList(list);
   }

   ConstantReturnType const* Evaluate(ast::Expr* node) {
      return ast::ExprEvaluator<ConstantReturnType const*>::Evaluate(node);
   }

protected:
   using Type = ast::Type;
   using BinaryOp = ast::exprnode::BinaryOp;
   using UnaryOp = ast::exprnode::UnaryOp;
   using DotOp = ast::exprnode::MemberAccess;
   using MethodOp = ast::exprnode::MethodInvocation;
   using NewOp = ast::exprnode::ClassInstanceCreation;
   using NewArrayOp = ast::exprnode::ArrayInstanceCreation;
   using ArrayAccessOp = ast::exprnode::ArrayAccess;
   using CastOp = ast::exprnode::Cast;
   using ExprValue = ast::exprnode::ExprValue;
   using ETy = ConstantReturnType const*;

   ETy mapValue(ExprValue& node) const override;
   ETy evalBinaryOp(BinaryOp& op, const ETy lhs, const ETy rhs) const override;
   ETy evalUnaryOp(UnaryOp& op, const ETy rhs) const override;
   ETy evalMemberAccess(DotOp& op, const ETy lhs, const ETy field) const override;
   ETy evalMethodCall(MethodOp& op, const ETy method,
                      const op_array& args) const override;
   ETy evalNewObject(NewOp& op, const ETy object,
                     const op_array& args) const override;
   ETy evalNewArray(NewArrayOp& op, const ETy type, const ETy size) const override;
   ETy evalArrayAccess(ArrayAccessOp& op, const ETy array,
                       const ETy index) const override;
   ETy evalCast(CastOp& op, const ETy type, const ETy value) const override;
   bool validatePop(ETy const& type) const override {
      (void) type;
      return true;
   }

private:
   BumpAllocator& alloc;
};

} // namespace semantic
