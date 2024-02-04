#pragma once

#include "AstNode.h"

namespace ast {

class CompoundStmt : public Stmt {
   std::vector<Stmt*> stmts;

public:
   CompoundStmt() {}
   CompoundStmt(std::vector<Stmt*> stmts) : stmts{stmts} {}
   std::ostream& print(std::ostream& os, int indentation = 0) const override;
};

} // namespace ast
