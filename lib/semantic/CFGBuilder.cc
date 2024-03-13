#include "semantic/CFGBuilder.h"

#include "ast/Stmt.h"

namespace semantic {

CFGBuilder::CFGInfo CFGBuilder::buildIteratively(const ast::Stmt* stmt,
                                                 CFGNode* parent) {
   using CFGInfo = CFGBuilder::CFGInfo;
   assert(stmt && "stmt is nullptr");
   CFGInfo node = {nullptr, {}};
   if(auto forStmt = dyn_cast<ast::ForStmt>(stmt)) {
      node = buildForStmt(forStmt);
   } else if(auto ifStmt = dyn_cast<ast::IfStmt>(stmt)) {
      node = buildIfStmt(ifStmt);
   } else if(auto declStmt = dyn_cast<ast::DeclStmt>(stmt)) {
      node = buildDeclStmt(declStmt);
   } else if(auto exprStmt = dyn_cast<ast::ExprStmt>(stmt)) {
      node = buildExprStmt(exprStmt);
   } else if(auto returnStmt = dyn_cast<ast::ReturnStmt>(stmt)) {
      node = buildReturnStmt(returnStmt);
   } else if(auto whileStmt = dyn_cast<ast::WhileStmt>(stmt)) {
      node = buildWhileStmt(whileStmt);
   } else if(auto blockStmt = dyn_cast<ast::BlockStatement>(stmt)) {
      node = buildBlockStmt(blockStmt);
      if(!node.head) {
         CFGNode* empty = alloc.new_object<CFGNode>(CFGNode::EmptyExpr{});
         return CFGInfo{empty, {empty}};
      }
   } else {
      CFGNode* empty = alloc.new_object<CFGNode>(CFGNode::EmptyExpr{});
      return CFGInfo{empty, {empty}};
   }

   assert(node.head && "node is nullptr");

   if(parent) {
      connectCFGNode(parent, node.head);
   }

   return node;
}

CFGBuilder::CFGInfo CFGBuilder::buildBlockStmt(
      const ast::BlockStatement* blockStmt) {
   CFGInfo node, parent, head;
   for(auto stmt : blockStmt->stmts()) {
      if(parent.head == nullptr) {
         parent = buildIteratively(stmt);
         head = parent;
      } else {
         node = buildIteratively(stmt);
         for(auto leaf : parent.leafs) {
            connectCFGNode(leaf, node.head);
         }
         parent = node;
      }
   }
   return CFGInfo{head.head, parent.leafs};
}

CFGBuilder::CFGInfo CFGBuilder::buildDeclStmt(const ast::DeclStmt* declStmt) {
   CFGNode* node = alloc.new_object<CFGNode>(declStmt->decl());
   return CFGInfo{node, {node}};
}

CFGBuilder::CFGInfo CFGBuilder::buildExprStmt(const ast::ExprStmt* exprStmt) {
   CFGNode* node = alloc.new_object<CFGNode>(exprStmt->expr());
   return CFGInfo{node, {node}};
}

CFGBuilder::CFGInfo CFGBuilder::buildForStmt(const ast::ForStmt* forStmt) {
   CFGNode *condition = nullptr;
   CFGInfo init;
   if(forStmt->init()) {
      init = buildIteratively(forStmt->init());
   } 
   // TODO(Larry): look into literal expression
   if(forStmt->condition()) {
      condition = alloc.new_object<CFGNode>(forStmt->condition());
   } else {
      condition = alloc.new_object<CFGNode>(CFGNode::EmptyExpr{});
   }
   if (init.head) connectCFGNode(init.head, condition);
   CFGInfo body = buildIteratively(forStmt->body(), condition);
   if(forStmt->update()) {
      CFGInfo update = buildIteratively(forStmt->update());
      for(auto leaf : body.leafs) {
         connectCFGNode(leaf, update.head);
      }
      connectCFGNode(update.head, condition); // update loops back to condition
   } else {
      for(auto leaf : body.leafs) {
         connectCFGNode(leaf, condition);
      }
   }
   if (init.head) return CFGInfo{init.head, {condition}};
   return CFGInfo{condition, {condition}};
}

CFGBuilder::CFGInfo CFGBuilder::buildIfStmt(const ast::IfStmt* ifStmt) {
   CFGNode* condition = alloc.new_object<CFGNode>(ifStmt->condition());
   // TODO(Larry): look into literal expression
   CFGInfo ifNode = buildIteratively(ifStmt->thenStmt(), condition);
   if(ifStmt->elseStmt()) {
      CFGInfo elseNode = buildIteratively(ifStmt->elseStmt(), condition);
      ifNode.leafs.insert(
            ifNode.leafs.end(), elseNode.leafs.begin(), elseNode.leafs.end());
   }
   return CFGInfo{condition, ifNode.leafs};
}

CFGBuilder::CFGInfo CFGBuilder::buildReturnStmt(
      const ast::ReturnStmt* returnStmt) {
   CFGNode* node = nullptr;
   if(returnStmt->expr() == nullptr) {
      node = alloc.new_object<CFGNode>(CFGNode::EmptyExpr{}, true);
   }
   node = alloc.new_object<CFGNode>(returnStmt->expr(), true);
   return CFGInfo{node, {node}};
}

CFGBuilder::CFGInfo CFGBuilder::buildWhileStmt(const ast::WhileStmt* whileStmt) {
   CFGNode* condition = alloc.new_object<CFGNode>(whileStmt->condition());
   // TODO(Larry): look into literal expression
   CFGInfo body = buildIteratively(whileStmt->body(), condition);
   for(auto leaf : body.leafs) {
      connectCFGNode(leaf, condition);
   }
   return CFGInfo{condition, {condition}};
}

void CFGBuilder::connectCFGNode(CFGNode* parent, CFGNode* child) {
   parent->children.push_back(child);
   child->parents.push_back(parent);
}

} // namespace semantic