#include "semantic/CFGBuilder.h"
#include "ast/Stmt.h"


namespace semantic {

CFGNode* CFGBuilder::buildIteratively(const ast::Stmt* stmt, CFGNode* parent) {
   if (auto forStmt = dyn_cast<ast::ForStmt>(stmt)) {
      return buildForStmt(forStmt, parent);
   } else if (auto ifStmt = dyn_cast<ast::IfStmt>(stmt)) {
      return buildIfStmt(ifStmt, parent);
   } else if (auto declStmt = dyn_cast<ast::DeclStmt>(stmt)) {
      return buildDeclStmt(declStmt, parent);
   } else if (auto exprStmt = dyn_cast<ast::ExprStmt>(stmt)) {
      return buildExprStmt(exprStmt, parent);
   } else if (auto returnStmt = dyn_cast<ast::ReturnStmt>(stmt)) {
      return buildReturnStmt(returnStmt, parent);
   } else if (auto whileStmt = dyn_cast<ast::WhileStmt>(stmt)) {
      return buildWhileStmt(whileStmt, parent);
   } else {
      return nullptr;
   }
}

CFGNode* CFGBuilder::buildDeclStmt(const ast::DeclStmt* declStmt, CFGNode* parent) {
   CFGNode* node = alloc.new_object<CFGNode>(declStmt->decl());
   if (parent) {
      parent->children.push_back(node);
      node->parents.push_back(parent);
   }
   for (auto child : declStmt->children()) {
      if (auto childStmt = dyn_cast<const ast::Stmt>(child)) {
         buildIteratively(childStmt, node);
      }
   }
   return node;
}

CFGNode* CFGBuilder::buildExprStmt(const ast::ExprStmt* exprStmt, CFGNode* parent) {
   // TODO: Implement
   return nullptr;
}

CFGNode* CFGBuilder::buildForStmt(const ast::ForStmt* forStmt, CFGNode* parent) {
   // TODO: Implement
   return nullptr;
}

CFGNode* CFGBuilder::buildIfStmt(const ast::IfStmt* ifStmt, CFGNode* parent) {
   // TODO: Implement
   return nullptr;
}

CFGNode* CFGBuilder::buildReturnStmt(const ast::ReturnStmt* returnStmt, CFGNode* parent) {
   // TODO: Implement
   return nullptr;
}

CFGNode* CFGBuilder::buildWhileStmt(const ast::WhileStmt* whileStmt, CFGNode* parent) {
   // TODO: Implement
   return nullptr;
}

} // namespace semantic