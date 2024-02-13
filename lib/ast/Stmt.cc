#include <ostream>
#include <string>

#include "ast/AST.h"

namespace ast {

using std::ostream;
using std::string;

std::ostream& CompoundStmt::print(std::ostream& os, int indentation) const {
   auto i1 = indent(indentation);
   if(stmts().empty()) {
      return os << i1 << "CompoundStmt {}\n";
   }
   os << i1 << "CompoundStmt {\n";
   for(auto stmt : stmts()) {
      stmt->print(os, indentation + 1) << "\n";
   }
   os << i1 << "}\n";
   return os;
}

int CompoundStmt::printDotNode(DotPrinter& dp) const {
   int id = dp.id();
   dp.startTLabel(id);
   dp.printTableSingleRow("CompoundStmt");
   dp.endTLabel();
   for(auto stmt : stmts_) {
      dp.printConnection(id, stmt->printDotNode(dp));
   }
   return id;
}

} // namespace ast
