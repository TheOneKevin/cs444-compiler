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

void ExprNode::dump() const {
   this->print(std::cerr);
}

void Expr::dump() const {
   this->print(std::cerr, 0);
}

void ExprNodeList::dump() const {
   for(auto node : nodes()) {
      node->print(std::cerr);
      std::cerr << " ";
   }
   std::cerr << "\n";
}

} // namespace ast