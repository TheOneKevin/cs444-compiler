#include <ast/AST.h>
#include <ast/Expr.h>

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

void Expr::dump() const { this->print(std::cerr, 0); }

/* ===--------------------------------------------------------------------=== */
// ExprNode Subclasses
/* ===--------------------------------------------------------------------=== */

void ExprNode::dump() const { this->print(std::cerr) << "\n"; }

void ExprNodeList::dump() const { print(std::cerr); }

std::ostream& ExprNodeList::print(std::ostream& os) const {
   for(auto node : nodes()) {
      node->print(os);
      os << " ";
   }
   os << "\n";
   return os;
}

void ExprNodeList::check_invariants() const {
   assert((!tail_ || tail_->next() == nullptr) &&
          "Tail node should not have a next node");
   assert(((head_ != nullptr) == (tail_ != nullptr)) &&
          "Head is null if and only if tail is null");
   assert(((head_ == nullptr) == (size_ == 0)) &&
          "Size should be 0 if and only if head is null");
}

namespace exprnode {

std::ostream& MemberName::print(std::ostream& os) const {
   return os << "(Member name:" << name_ << ")";
}

std::ostream& MethodName::print(std::ostream& os) const {
   return os << "(Method name:" << name() << ")";
}

std::ostream& ThisNode::print(std::ostream& os) const { return os << "(THIS)"; }

std::ostream& TypeNode::print(std::ostream& os) const {
   os << "(Type: ";
   type()->print(os);
   return os << ")";
}

LiteralNode::LiteralNode(BumpAllocator& alloc, std::string_view value,
                         ast::BuiltInType* type)
      : ExprValue{reinterpret_cast<ast::Type*>(type)}, value_{value, alloc} {}

std::ostream& LiteralNode::print(std::ostream& os) const {
   // TODO(kevin): re-implement this
   return os << "(Literal)";
}

ast::BuiltInType const* LiteralNode::builtinType() const {
   return cast<ast::BuiltInType>(type());
}

std::ostream& MemberAccess::print(std::ostream& os) const {
   return os << "MemberAccess";
}

std::ostream& MethodInvocation::print(std::ostream& os) const {
   return os << "MethodInvocation(" << nargs() - 1 << ")";
}

std::ostream& ClassInstanceCreation::print(std::ostream& os) const {
   return os << "(ClassInstanceCreation args: " << std::to_string(nargs()) << ")";
}

std::ostream& ArrayInstanceCreation::print(std::ostream& os) const {
   return os << "ArrayInstanceCreation";
}

std::ostream& ArrayAccess::print(std::ostream& os) const {
   return os << "ArrayAccess";
}

std::ostream& Cast::print(std::ostream& os) const { return os << "Cast"; }

std::ostream& UnaryOp::print(std::ostream& os) const {
   return os << OpType_to_string(type, "(Unknown unary op)");
}

std::ostream& BinaryOp::print(std::ostream& os) const {
   return os << OpType_to_string(type, "(Unknown binary op)");
}

} // namespace exprnode

} // namespace ast