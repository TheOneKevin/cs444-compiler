#include "semantic/CFGBuilder.h"

#include <cassert>

#include "ast/AstNode.h"
#include "ast/Stmt.h"
#include "utils/Utils.h"

namespace semantic {

CFGBuilder::CFGInfo CFGBuilder::buildIteratively(const ast::Stmt* stmt,
                                                 CFGNode* parent) {
   using CFGInfo = CFGBuilder::CFGInfo;
   assert(stmt && "stmt is nullptr");
   CFGInfo node = {alloc, nullptr};
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
         CFGNode* empty = alloc.new_object<CFGNode>(alloc, CFGNode::EmptyExpr{});
         return CFGInfo{alloc, empty, empty};
      }
   } else {
      CFGNode* empty = alloc.new_object<CFGNode>(alloc, CFGNode::EmptyExpr{});
      return CFGInfo{alloc, empty, empty};
   }

   assert(node.head && "node is nullptr");

   if(parent) {
      connectCFGNode(parent, node.head);
   }

   return node;
}

CFGBuilder::CFGInfo CFGBuilder::buildBlockStmt(
      const ast::BlockStatement* blockStmt) {
   CFGInfo node = {alloc, nullptr}, parent = {alloc, nullptr},
           head = {alloc, nullptr};
   for(auto stmt : blockStmt->stmts()) {
      if(parent.head == nullptr) {
         parent = buildIteratively(stmt);
         head = parent;
      } else {
         node = buildIteratively(stmt);
         connectLeafsToChild(parent, node.head);
         parent = node;
      }
   }
   CFGInfo info = CFGInfo{alloc, head.head};
   for(auto leaf : parent.leafs) {
      info.leafs.push_back(leaf);
   }
   return info;
}

CFGBuilder::CFGInfo CFGBuilder::buildDeclStmt(const ast::DeclStmt* declStmt) {
   CFGNode* node = alloc.new_object<CFGNode>(alloc, declStmt->decl());
   return CFGInfo{alloc, node, node};
}

CFGBuilder::CFGInfo CFGBuilder::buildExprStmt(const ast::ExprStmt* exprStmt) {
   CFGNode* node = alloc.new_object<CFGNode>(alloc, exprStmt->expr());
   return CFGInfo{alloc, node, node};
}

CFGBuilder::CFGInfo CFGBuilder::buildForStmt(const ast::ForStmt* forStmt) {
   CFGNode* condition = nullptr;
   CFGInfo init = {alloc, nullptr};
   if(forStmt->init()) {
      init = buildIteratively(forStmt->init());
   }

   if(forStmt->condition()) {
      condition = alloc.new_object<CFGNode>(alloc, forStmt->condition());
      ConstantReturnType const* ret =
            constTypeResolver->Evaluate(forStmt->condition());

      if(ret->constantType == ConstantReturnType::type::BOOL) {
         if(ret->value == 0) {
            diag.ReportError(forStmt->condition()->location())
                  << "Unreachable statement in for loop body";
         } else {
            assert(ret->value == 1 && "invalid boolean value");
         }
      }
   } else {
      condition = alloc.new_object<CFGNode>(alloc, CFGNode::EmptyExpr{});
   }

   if(init.head) connectCFGNode(init.head, condition);
   CFGInfo body = buildIteratively(forStmt->body(), condition);
   if(forStmt->update()) {
      CFGInfo update = buildIteratively(forStmt->update());
      connectLeafsToChild(body, update.head);
      connectLeafsToChild(update, condition);
   } else {
      connectLeafsToChild(body, condition);
   }
   if(init.head) return CFGInfo{alloc, init.head, condition};
   return CFGInfo{alloc, condition, condition};
}

CFGBuilder::CFGInfo CFGBuilder::buildIfStmt(const ast::IfStmt* ifStmt) {
   CFGNode* condition = alloc.new_object<CFGNode>(alloc, ifStmt->condition());

   CFGInfo info = {alloc, condition};
   // by java rules, we allow if statements to have a constant condition
   CFGInfo ifNode = buildIteratively(ifStmt->thenStmt(), condition);
   info.leafs.insert(info.leafs.end(), ifNode.leafs.begin(), ifNode.leafs.end());
   if(ifStmt->elseStmt()) {
      CFGInfo elseNode = buildIteratively(ifStmt->elseStmt(), condition);
      info.leafs.insert(
            info.leafs.end(), elseNode.leafs.begin(), elseNode.leafs.end());
   } else {
      CFGNode* empty = alloc.new_object<CFGNode>(alloc, CFGNode::EmptyExpr{});
      info.leafs.push_back(empty);
      connectCFGNode(condition, empty);
   }
   return info;
}

CFGBuilder::CFGInfo CFGBuilder::buildReturnStmt(
      const ast::ReturnStmt* returnStmt) {
   CFGNode* node = nullptr;
   if(returnStmt->expr() == nullptr) {
      node = alloc.new_object<CFGNode>(alloc, CFGNode::EmptyExpr{}, true);
   } else {
      node = alloc.new_object<CFGNode>(alloc, returnStmt->expr(), true);
   }
   return CFGInfo{alloc, node, node};
}

CFGBuilder::CFGInfo CFGBuilder::buildWhileStmt(const ast::WhileStmt* whileStmt) {
   CFGNode* condition = alloc.new_object<CFGNode>(alloc, whileStmt->condition());
   ConstantReturnType const* ret =
         constTypeResolver->Evaluate(whileStmt->condition());
   CFGNode* leafNode = condition;
   if(ret->constantType == ConstantReturnType::type::BOOL) {
      if(ret->value == 0) {
         diag.ReportError(whileStmt->condition()->location())
               << "Unreachable statement in while loop body";
      } else if(ret->value == 1) {
         condition->setVisited(true);
         leafNode = nullptr;
      } else {
         assert(ret->value == 1 && "invalid boolean value");
         leafNode = nullptr;
      }
   }

   if(whileStmt->body()) {
      CFGInfo body = buildIteratively(whileStmt->body(), condition);
      connectLeafsToChild(body, condition);
   }
   if(leafNode) {
      return CFGInfo{alloc, condition, leafNode};
   }
   return CFGInfo{alloc, condition};
}

void CFGBuilder::connectCFGNode(CFGNode* parent, CFGNode* child) {
   parent->children.push_back(child);
   child->parents.push_back(parent);
}

void CFGBuilder::connectLeafsToChild(CFGInfo parent, CFGNode* child) {
   for(auto leaf : parent.leafs) {
      connectCFGNode(leaf, child);
   }
   if(child->parents.empty()) {
      if(child->location()) {
         diag.ReportError(child->location().value()) << "Unreachable statement";
      } else {
         diag.ReportError(parent.head->location().value()) << "Unreachable statement";
      }
   }
}

} // namespace semantic