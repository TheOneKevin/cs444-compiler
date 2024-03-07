#pragma once

#include "ast/AstNode.h"
#include "ast/ExprNode.h"
#include "diagnostics/Location.h"

namespace ast {

class Expr {
public:
   Expr(ExprNodeList rpn_ops, SourceRange loc, ScopeID const* scope)
         : rpn_ops{rpn_ops}, loc_{loc}, scope_{scope} {}
   std::ostream& print(std::ostream& os, int indentation) const;
   int printDotNode(DotPrinter& dp) const;
   auto nodes() const { return rpn_ops.nodes(); }
   auto list() const { return rpn_ops; }
   auto location() const { return loc_; }
   void dump() const;
   auto mut_nodes() { return rpn_ops.mut_nodes(); }
   void replace(ExprNodeList new_list) { rpn_ops = new_list; }
   auto scope() const { return scope_; }
   void setScope(ScopeID const* scope) { scope_ = scope; }

private:
   ExprNodeList rpn_ops;
   SourceRange loc_;
   ScopeID const* scope_;
};

} // namespace ast
