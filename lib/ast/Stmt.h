#pragma once

#include <ranges>

#include "AstNode.h"

namespace ast {

class CompoundStmt : public Stmt {
public:
   CompoundStmt() {}
   CompoundStmt(array_ref<Stmt*> stmts) : stmts_{stmts} {}
   auto stmts() const { return std::views::all(stmts_); }

   std::ostream& print(std::ostream& os, int indentation = 0) const override;
   int printDotNode(DotPrinter& dp) const override;

private:
   pmr_vector<Stmt*> stmts_;
};

} // namespace ast
