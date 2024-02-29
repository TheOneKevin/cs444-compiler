#include <ast/AST.h>
#include <ast/Expr.h>

#include <concepts>
#include <ostream>

#include "ast/ExprNode.h"

namespace ast {

std::ostream& Expr::print(std::ostream& os, int indentation) const {
   if(indentation >= 0) {
      os << AstNode::indent(indentation);
      os << "Expr {\n";
   }
   os << AstNode::indent(indentation + 1);
   char state = '(';
   for(const auto op : rpn_ops.nodes()) {
      if(indentation >= 0) {
         std::ostringstream oss;
         op->print(oss);
         char cur = (oss.str().begin()[0] == '(') ? '(' : '.';
         if(cur == state) {
            os << oss.str() << " ";
         } else {
            os << "\n" << AstNode::indent(indentation + 1) << oss.str() << " ";
         }
         state = cur;
      } else {
         op->print(os);
         os << "\n";
      }
   }
   if(indentation >= 0) {
      os << "\n" << AstNode::indent(indentation) << "}\n";
   }
   return os;
}

int Expr::printDotNode(DotPrinter& dp) const {
   std::ostringstream os;
   for(const auto op : rpn_ops.nodes()) {
      op->print(os);
      if(os.str().ends_with(")")) {
         os << "\n";
      } else {
         os << " ";
      }
   }
   dp.sanitize(os.str());
   return -1;
}

template <typename T>
   requires std::movable<T>
void ExprEvaluator<T>::Evaluate(Expr* expr) {
   using namespace ast::exprnode;

   std::pmr::vector<T> op_args;
   const auto getArgs = [&op_args, this](ExprOp* args) {
      op_args.clear();
      for(int i = 0; i < args->nargs(); ++i) {
         op_args.push_back(op_stack_.top());
         op_stack_.pop();
      }
   };

   // Evaluate the RPN expression
   for(auto const* nodes : expr->nodes()) {
      if(auto* value = dynamic_cast<ExprValue const*>(nodes)) {
         op_stack_.push(mapValue(*value));
      } else if(auto* unary = dynamic_cast<UnaryOp const*>(nodes)) {
         auto rhs = op_stack_.top();
         op_stack_.pop();
         op_stack_.push(evalUnaryOp(*unary, rhs));
      } else if(auto* binary = dynamic_cast<BinaryOp const*>(nodes)) {
         auto rhs = op_stack_.top();
         op_stack_.pop();
         auto lhs = op_stack_.top();
         op_stack_.pop();
         op_stack_.push(evalBinaryOp(*binary, lhs, rhs));
      } else if(auto* member = dynamic_cast<MemberAccess const*>(nodes)) {
         auto field = op_stack_.top();
         op_stack_.pop();
         auto lhs = op_stack_.top();
         op_stack_.pop();
         op_stack_.push(evalMemberAccess(lhs, field));
      } else if(auto* method = dynamic_cast<MethodInvocation const*>(nodes)) {
         getArgs(method);
         op_stack_.push(evalMethodCall(op_args));
      } else if(auto* new_object =
                      dynamic_cast<ClassInstanceCreation const*>(nodes)) {
         getArgs(new_object);
         op_stack_.push(evalNewObject(op_args));
      } else if(auto* new_array =
                      dynamic_cast<ArrayInstanceCreation const*>(nodes)) {
         auto size = op_stack_.top();
         op_stack_.pop();
         auto type = op_stack_.top();
         op_stack_.pop();
         op_stack_.push(evalNewArray(type, size));
      } else if(auto* array_access = dynamic_cast<ArrayAccess const*>(nodes)) {
         auto index = op_stack_.top();
         op_stack_.pop();
         auto array = op_stack_.top();
         op_stack_.pop();
         op_stack_.push(evalArrayAccess(array, index));
      } else if(auto* cast = dynamic_cast<Cast const*>(nodes)) {
         auto value = op_stack_.top();
         op_stack_.pop();
         auto type = op_stack_.top();
         op_stack_.pop();
         op_stack_.push(evalCast(type, value));
      }
   }
}

} // namespace ast