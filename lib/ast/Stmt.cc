#include <ostream>
#include <string>

#include "ast/AST.h"
namespace ast {

using std::ostream;
using std::string;

std::pair<int, int> printStmtSubgraph(utils::DotPrinter& dp, ast::Stmt* stmt) {
   int firstId = -1;
   int subgraphId = -1;
   if(auto block = dyn_cast_or_null<ast::BlockStatement*>(stmt)) {
      subgraphId = dp.id();
      dp.startSubgraph(subgraphId);
      dp.print("label=\"Statement body\"");
      dp.print("color=lightblue");
      firstId = printDotNodeList(dp, block->stmts());
      if(firstId == -1) {
         firstId = dp.id();
         dp.printLabel(firstId, "Empty Body");
      }
      dp.endSubgraph();
   } else {
      firstId = stmt->printDotNode(dp);
   }
   return {firstId, subgraphId};
}

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
   dp.printLabel(id, "BlockStatement");
   for(auto stmt : stmts_) {
      dp.printConnection(id, stmt->printDotNode(dp));
   }
   return id;
}

// DeclStmt ////////////////////////////////////////////////////////////////////

std::ostream& DeclStmt::print(std::ostream& os, int indentation) const {
   (void)indentation;
   return os;
}

int DeclStmt::printDotNode(DotPrinter& dp) const {
   return decl_->printDotNode(dp);
}

// ExprStmt ////////////////////////////////////////////////////////////////////

std::ostream& ExprStmt::print(std::ostream& os, int indentation) const {
   expr_->print(os, indentation);
   return os;
}

int ExprStmt::printDotNode(DotPrinter& dp) const {
   auto id = dp.id();
   std::ostringstream expr;
   expr_->print(expr, -1);
   dp.startTLabel(id);
   dp.printTableSingleRow("ExprStmt", {"bgcolor", "lightblue"});
   dp.printTableSingleRow(expr.str(), {"balign", "left"});
   dp.endTLabel();
   return id;
}

// IfStmt //////////////////////////////////////////////////////////////////////

std::ostream& IfStmt::print(std::ostream& os, int indentation) const {
   // Implementation for IfStmt print
   (void)indentation;
   return os;
}

int IfStmt::printDotNode(DotPrinter& dp) const {
   auto id = dp.id();
   std::ostringstream expr;
   condition_->print(expr, -1);
   dp.startTLabel(id);
   dp.printTableSingleRow("IfStmt", {"bgcolor", "lightblue"});
   dp.printTableDoubleRow(
         "condition", expr.str(), {"port", "condition"}, {"balign", "left"});
   dp.printTableDoubleRow("then", "else", {"port", "then"}, {"port", "else"});
   dp.endTLabel();
   // True branch
   auto ids = printStmtSubgraph(dp, thenStmt_);
   if(ids.second != -1)
      dp.printConnection(id, ":then:c", ids.first, ids.second);
   else
      dp.printConnection(id, ":then:c", ids.first);
   // False branch
   if(elseStmt_) {
      ids = printStmtSubgraph(dp, elseStmt_);
      if(ids.second != -1)
         dp.printConnection(id, ":else:c", ids.first, ids.second);
      else
         dp.printConnection(id, ":else:c", ids.first);
   }
   return id;
}

// WhileStmt ///////////////////////////////////////////////////////////////////

std::ostream& WhileStmt::print(std::ostream& os, int indentation) const {
   // Implementation for WhileStmt print
   (void)indentation;
   return os;
}

int WhileStmt::printDotNode(DotPrinter& dp) const {
   auto id = dp.id();
   dp.startTLabel(id);
   dp.printTableSingleRow("WhileStmt", {"bgcolor", "lightblue"});
   dp.printTableDoubleRow(
         "condition", "body", {"port", "condition"}, {"port", "body"});
   dp.endTLabel();
   // dp.printConnection(id, ":condition", condition_->printDotNode(dp));
   auto ids = printStmtSubgraph(dp, body_);
   if(ids.second != -1)
      dp.printConnection(id, ":body", ids.first, ids.second);
   else
      dp.printConnection(id, ":body", ids.first);
   return id;
}

// ForStmt /////////////////////////////////////////////////////////////////////

std::ostream& ForStmt::print(std::ostream& os, int indentation) const {
   // Implementation for ForStmt print
   (void)indentation;
   return os;
}

int ForStmt::printDotNode(DotPrinter& dp) const {
   auto id = dp.id();
   std::ostringstream expr;
   if(condition_) condition_->print(expr, -1);
   dp.startTLabel(id);
   dp.printTableSingleRow("ForStmt", {"bgcolor", "lightblue"});
   dp.printTableDoubleRow(
         "condition", expr.str(), {"port", "condition"}, {"balign", "left"});
   dp.printTableTripleRow("init",
                          "update",
                          "body",
                          {"port", "init"},
                          {"port", "update"},
                          {"port", "body"});
   dp.endTLabel();
   if(init_) dp.printConnection(id, ":init", init_->printDotNode(dp));
   if(update_) dp.printConnection(id, ":update", update_->printDotNode(dp));
   auto ids = printStmtSubgraph(dp, body_);
   if(ids.second != -1)
      dp.printConnection(id, ":body", ids.first, ids.second);
   else
      dp.printConnection(id, ":body", ids.first);
   return id;
}

// ReturnStmt //////////////////////////////////////////////////////////////////

std::ostream& ReturnStmt::print(std::ostream& os, int indentation) const {
   (void)indentation;
   return os;
}

int ReturnStmt::printDotNode(DotPrinter& dp) const {
   auto id = dp.id();
   dp.printLabel(id, "ReturnStmt");
   return id;
}

// NullStmt ////////////////////////////////////////////////////////////////////

std::ostream& NullStmt::print(std::ostream& os, int indentation) const {
   (void)indentation;
   return os;
}

int NullStmt::printDotNode(DotPrinter& dp) const {
   auto id = dp.id();
   dp.printLabel(id, "NullStmt");
   return id;
}

} // namespace ast
