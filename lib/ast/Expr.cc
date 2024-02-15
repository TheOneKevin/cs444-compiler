#include "ast/Expr.h"

#include <ostream>

#include "ast/AST.h"

namespace ast {

std::ostream& Expr::print(std::ostream& os, int indentation) const {
   os << AstNode::indent(indentation);
   for(const auto& op : rpn_ops) {
      op->print(os);
      os << " ";
   }
   os << "\n";
   return os;
}

int Expr::printDotNode(DotPrinter& dp) const { return dp.id(); }

} // namespace ast