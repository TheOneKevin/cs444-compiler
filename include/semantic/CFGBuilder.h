#pragma once

#include <unordered_map>
#include <variant>
#include <vector>

#include "ast/AstNode.h"
#include "ast/Decl.h"
#include "ast/DeclContext.h"
#include "ast/Stmt.h"
#include "semantic/Semantic.h"
#include "utils/BumpAllocator.h"
#include "utils/DotPrinter.h"

namespace semantic {

using utils::DotPrinter;
class CFGNode {
   struct EmptyExpr {};
   friend class CFGBuilder;
   std::pmr::vector<CFGNode*> children;
   std::pmr::vector<CFGNode*> parents;
   std::variant<const ast::Expr*, const ast::VarDecl*, EmptyExpr> data;
   bool isReturn;

public:
   CFGNode(std::variant<const ast::Expr*, const ast::VarDecl*, EmptyExpr> data,
           bool isReturn = false)
         : data(data), isReturn(isReturn) {
      children = std::pmr::vector<CFGNode*>();
      parents = std::pmr::vector<CFGNode*>();
   }

   std::ostream& printDot(std::ostream& os) const {
      std::pmr::unordered_map<const CFGNode*, int> visited;
      DotPrinter dp{os};
      dp.startGraph();
      dp.print("compound=true;");
      printDotNode(dp, visited);
      dp.endGraph();
      return os;
   }

private:
   int printDotNode(utils::DotPrinter& dp,
                    std::pmr::unordered_map<const CFGNode*, int>& visited) const {
      if(visited.count(this) > 0) {
         return visited[this];
      }
      int id = dp.id();
      dp.startTLabel(id);
      if(isReturn) {
         dp.printTableSingleRow("Return Statement", {"bgcolor", "lightblue"});
      }
      if(std::holds_alternative<const ast::Expr*>(data)) {
         std::ostringstream expr;
         std::get<const ast::Expr*>(data)->print(expr, -1);
         dp.printTableSingleRow("Expr", {"bgcolor", "lightblue"});
         dp.printTableDoubleRow(
               "expr", expr.str(), {"port", "condition"}, {"balign", "left"});
      } else if(std::holds_alternative<const ast::VarDecl*>(data)) {
         auto name = std::get<const ast::VarDecl*>(data)->name();
         dp.printTableSingleRow("VarDecl", {"bgcolor", "lightblue"});
         dp.printTableDoubleRow(
               "expr", name, {"port", "condition"}, {"balign", "left"});
      } else {
         dp.printTableSingleRow("Empty Expr", {"bgcolor", "lightblue"});
      }
      dp.endTLabel();
      visited[this] = id;
      for(auto child : children) {
         dp.printConnection(id, child->printDotNode(dp, visited));
      }
      return id;
   }
};

class CFGBuilder {
   using Heap = std::pmr::memory_resource;
   struct CFGInfo {
      CFGNode* head;
      std::pmr::vector<CFGNode*> leafs;
   };

private:
   CFGInfo buildIteratively(const ast::Stmt* stmt, CFGNode* parent = nullptr);
   CFGInfo buildForStmt(const ast::ForStmt* forStmt);
   CFGInfo buildIfStmt(const ast::IfStmt* ifStmt);
   CFGInfo buildDeclStmt(const ast::DeclStmt* declStmt);
   CFGInfo buildExprStmt(const ast::ExprStmt* exprStmt);
   CFGInfo buildReturnStmt(const ast::ReturnStmt* returnStmt);
   CFGInfo buildWhileStmt(const ast::WhileStmt* whileStmt);
   CFGInfo buildBlockStmt(const ast::BlockStatement* blockStmt);

   void connectCFGNode(CFGNode* parent, CFGNode* child);

public:
   CFGBuilder(Heap* heap, ast::Semantic& sema)
         : alloc{heap}, heap{heap}, sema(sema) {}
   CFGNode* build(const ast::Stmt* stmt) { return buildIteratively(stmt).head; }

private:
   mutable BumpAllocator alloc;
   Heap* heap;
   ast::Semantic& sema;
};
} // namespace semantic