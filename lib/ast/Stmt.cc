#include <ostream>
#include <string>

#include "ast/AST.h"
namespace ast {

using std::ostream;
using std::string;

// BlockStatement //////////////////////////////////////////////////////////////

std::ostream& BlockStatement::print(std::ostream& os, int indentation) const {
   auto i1 = indent(indentation);
   if(stmts().empty()) {
      return os << i1 << "BlockStatement {}\n";
   }
   os << i1 << "BlockStatement {\n";
   for(auto stmt : stmts()) {
      stmt->print(os, indentation + 1) << "\n";
   }
   os << i1 << "}\n";
   return os;
}

int BlockStatement::printDotNode(DotPrinter& dp) const {
   int id = dp.id();
   dp.startTLabel(id);
   dp.printTableSingleRow("BlockStatement");
   dp.endTLabel();
   for(auto stmt : stmts_) {
      dp.printConnection(id, stmt->printDotNode(dp));
   }
   return id;
}

// DeclStmt ////////////////////////////////////////////////////////////////////

std::ostream& DeclStmt::print(std::ostream& os, int indentation) const {
   return os;
}

int DeclStmt::printDotNode(DotPrinter& dp) const {
   return dp.id();
}

// ExprStmt ////////////////////////////////////////////////////////////////////

std::ostream& ExprStmt::print(std::ostream& os, int indentation) const {
   expr_->print(os, indentation);
   return os;
}

int ExprStmt::printDotNode(DotPrinter& dp) const {
   return dp.id();
}

// IfStmt //////////////////////////////////////////////////////////////////////

std::ostream& IfStmt::print(std::ostream& os, int indentation) const {
   // Implementation for IfStmt print
   return os;
}

int IfStmt::printDotNode(DotPrinter& dp) const {
   // Implementation for IfStmt printDotNode
   return dp.id();
}

// WhileStmt ///////////////////////////////////////////////////////////////////

std::ostream& WhileStmt::print(std::ostream& os, int indentation) const {
   // Implementation for WhileStmt print
   return os;
}

int WhileStmt::printDotNode(DotPrinter& dp) const {
   // Implementation for WhileStmt printDotNode
   return dp.id();
}

// ForStmt /////////////////////////////////////////////////////////////////////

std::ostream& ForStmt::print(std::ostream& os, int indentation) const {
   // Implementation for ForStmt print
   return os;
}

int ForStmt::printDotNode(DotPrinter& dp) const {
   // Implementation for ForStmt printDotNode
   return dp.id();
}

// ReturnStmt //////////////////////////////////////////////////////////////////

std::ostream& ReturnStmt::print(std::ostream& os, int indentation) const {
   return os;
}

int ReturnStmt::printDotNode(DotPrinter& dp) const {
   return dp.id();
}

// NullStmt ////////////////////////////////////////////////////////////////////

std::ostream& NullStmt::print(std::ostream& os, int indentation) const {
   return os;
}

int NullStmt::printDotNode(DotPrinter& dp) const {
   return dp.id();
}

} // namespace ast
