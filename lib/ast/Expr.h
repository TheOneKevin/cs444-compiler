#pragma once

#include <concepts>
#include <stack>
#include <vector>

#include "ast/AstNode.h"
#include "ast/ExprNode.h"
#include "diagnostics/Location.h"

namespace ast {

class Expr {
   ExprNodeList rpn_ops;
   SourceRange loc_;

public:
   Expr(ExprNodeList rpn_ops, SourceRange loc) : rpn_ops{rpn_ops}, loc_{loc} {}
   std::ostream& print(std::ostream& os, int indentation) const;
   int printDotNode(DotPrinter& dp) const;
   auto nodes() const { return rpn_ops.nodes(); }
   auto location() const { return loc_; }
};

template <typename T>
   requires std::movable<T>
class ExprEvaluator {
protected:
   using op_array = std::pmr::vector<T>;

   virtual T mapValue(exprnode::ExprValue const& node) const = 0;

   virtual T evalBinaryOp(const exprnode::BinaryOp op, const T lhs, const T rhs) const = 0;
   virtual T evalUnaryOp(const exprnode::UnaryOp op, const T rhs) const = 0;

   virtual T evalMemberAccess(const T lhs, const T field) const = 0;
   virtual T evalMethodCall(const T method, const op_array& args) const = 0;
   virtual T evalNewObject(const T object, const op_array& args) const = 0;
   virtual T evalNewArray(const T type, const T size) const = 0;
   virtual T evalArrayAccess(const T array, const T index) const = 0;
   virtual T evalCast(const T type, const T value) const = 0;

public:
   void Evaluate(Expr* expr);

private:
   std::stack<T> op_stack_;
};

} // namespace ast
