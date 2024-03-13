#include "semantic/CFGBuilder.h"
#include "ast/Stmt.h"


namespace semantic {

CFGNode* CFGBuilder::buildIteratively(const ast::Stmt* stmt, CFGNode* parent) {
   CFGNode* node = nullptr;
   if (auto forStmt = dyn_cast<ast::ForStmt>(stmt)) {
      node = buildForStmt(forStmt);
   } else if (auto ifStmt = dyn_cast<ast::IfStmt>(stmt)) {
      node = buildIfStmt(ifStmt);
   } else if (auto declStmt = dyn_cast<ast::DeclStmt>(stmt)) {
      node = buildDeclStmt(declStmt);
   } else if (auto exprStmt = dyn_cast<ast::ExprStmt>(stmt)) {
      node = buildExprStmt(exprStmt);
   } else if (auto returnStmt = dyn_cast<ast::ReturnStmt>(stmt)) {
      node = buildReturnStmt(returnStmt);
   } else if (auto whileStmt = dyn_cast<ast::WhileStmt>(stmt)) {
      node = buildWhileStmt(whileStmt);
   } else {
      return nullptr;
   }
   if (parent) {
      parent->children.push_back(node);
      node->parents.push_back(parent);
   }
   return node;
}

CFGNode* CFGBuilder::buildDeclStmt(const ast::DeclStmt* declStmt) {
   CFGNode* node = alloc.new_object<CFGNode>(declStmt->decl());
   for (auto child : declStmt->children()) {
      if (auto childStmt = dyn_cast<const ast::Stmt>(child)) {
         buildIteratively(childStmt, node);
      }
   }
   return node;
}

CFGNode* CFGBuilder::buildExprStmt(const ast::ExprStmt* exprStmt) {
   CFGNode* node = alloc.new_object<CFGNode>(exprStmt->expr());
   for (auto child : exprStmt->children()) {
      if (auto childStmt = dyn_cast<const ast::Stmt>(child)) {
         buildIteratively(childStmt, node);
      }
   }
   return node;
}

CFGNode* CFGBuilder::buildForStmt(const ast::ForStmt* forStmt) {
   // TODO: Implement
   return nullptr;
}

CFGNode* CFGBuilder::buildIfStmt(const ast::IfStmt* ifStmt) {
   CFGNode* condition = alloc.new_object<CFGNode>(ifStmt->condition());
   // TODO(Larry): look into literal expression
   buildIteratively(ifStmt->thenStmt(), condition);
   buildIteratively(ifStmt->elseStmt(), condition);
   return condition;
}

CFGNode* CFGBuilder::buildReturnStmt(const ast::ReturnStmt* returnStmt) {
   // TODO: Implement
   return nullptr;
}

CFGNode* CFGBuilder::buildWhileStmt(const ast::WhileStmt* whileStmt) {
   // TODO: Implement
   return nullptr;
}

} // namespace semantic