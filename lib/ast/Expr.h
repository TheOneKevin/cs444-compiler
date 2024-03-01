#pragma once

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
   auto list() const { return rpn_ops; }
   auto location() const { return loc_; }
   void dump() const;
   // FIXME(kevin): Temporary, remove later
   void clear() { rpn_ops = ExprNodeList{}; }
};

} // namespace ast
