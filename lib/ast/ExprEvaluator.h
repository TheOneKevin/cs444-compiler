#pragma once

#include <concepts>
#include <stack>
#include <vector>

#include "ast/Expr.h"

namespace ast {
template <typename T>
   requires std::movable<T>
class ExprEvaluator {
protected:
   using op_array = std::pmr::vector<T>;

   virtual T mapValue(exprnode::ExprValue& node) const = 0;

   virtual T evalBinaryOp(exprnode::BinaryOp& op, const T lhs,
                          const T rhs) const = 0;
   virtual T evalUnaryOp(exprnode::UnaryOp& op, const T rhs) const = 0;

   virtual T evalMemberAccess(const T lhs, const T field) const = 0;
   virtual T evalMethodCall(const T method, const op_array& args) const = 0;
   virtual T evalNewObject(const T object, const op_array& args) const = 0;
   virtual T evalNewArray(const T type, const T size) const = 0;
   virtual T evalArrayAccess(const T array, const T index) const = 0;
   virtual T evalCast(const T type, const T value) const = 0;

   /**
    * @brief Gets the location of the argument at the given index.
    *
    * @param arg_index The index of the argument.
    * @return SourceLocation
    */
   SourceRange argLocation(int arg_index) {
      // FIXME(kevin): Implement this
      (void)arg_index;
      return SourceRange{};
   }

public:
   /**
    * @brief Evaluates the given expression.
    * @param expr The expression to evaluate.
    */
   T Evaluate(Expr* expr) { return Evaluate(expr->list()); }

   /**
    * @brief Evaluates the given subexpression.
    * @param subexpr The list of expression nodes to evaluate.
    */
   T Evaluate(ExprNodeList subexpr) {
      using namespace ast::exprnode;

      std::pmr::vector<T> op_args;
      const auto getArgs = [&op_args, this](int nargs) {
         op_args.clear();
         for(int i = 0; i < nargs; ++i) {
            op_args.push_back(popSafe());
         }
      };

      // Clear the stack
      while(!op_stack_.empty()) popSafe();

      // Lock all the nodes
      for(auto const* nodes : subexpr.nodes()) {
         nodes->const_lock();
      }

      // Evaluate the RPN expression
      auto* node = subexpr.mut_head();
      for(size_t i = 0; i < subexpr.size(); ++i) {
         // We grab the next node because we will unlock the current node
         auto next_node = node->mut_next();
         node->const_unlock();
         if(auto* value = dynamic_cast<ExprValue*>(node)) {
            op_stack_.push(mapValue(*value));
         } else if(auto* unary = dynamic_cast<UnaryOp*>(node)) {
            auto rhs = popSafe();
            op_stack_.push(evalUnaryOp(*unary, rhs));
         } else if(auto* binary = dynamic_cast<BinaryOp*>(node)) {
            auto rhs = popSafe();
            auto lhs = popSafe();
            op_stack_.push(evalBinaryOp(*binary, lhs, rhs));
         } else if(auto* member = dynamic_cast<MemberAccess const*>(node)) {
            auto field = popSafe();
            auto lhs = popSafe();
            op_stack_.push(evalMemberAccess(lhs, field));
         } else if(auto* method = dynamic_cast<MethodInvocation const*>(node)) {
            if(method->nargs() > 1)
               getArgs(method->nargs()-1);
            auto method_name = popSafe();
            op_stack_.push(evalMethodCall(method_name, op_args));
         } else if(auto* new_object =
                         dynamic_cast<ClassInstanceCreation const*>(node)) {
            if(new_object->nargs() > 1)
               getArgs(new_object->nargs()-1);
            auto type = popSafe();
            op_stack_.push(evalNewObject(type, op_args));
         } else if(auto* new_array =
                         dynamic_cast<ArrayInstanceCreation const*>(node)) {
            auto size = popSafe();
            auto type = popSafe();
            op_stack_.push(evalNewArray(type, size));
         } else if(auto* array_access = dynamic_cast<ArrayAccess const*>(node)) {
            auto index = popSafe();
            auto array = popSafe();
            op_stack_.push(evalArrayAccess(array, index));
         } else if(auto* cast = dynamic_cast<Cast const*>(node)) {
            auto value = popSafe();
            auto type = popSafe();
            op_stack_.push(evalCast(type, value));
         }
         node = next_node;
      }

      // Return the result
      auto result = popSafe();
      assert(op_stack_.empty() && "Stack not empty after evaluation");
      return result;
   }

private:
   inline T popSafe() {
      assert(!op_stack_.empty() && "Stack underflow");
      T value = op_stack_.top();
      op_stack_.pop();
      return value;
   }

private:
   std::stack<T> op_stack_;
};
} // namespace ast
