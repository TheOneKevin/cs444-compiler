#pragma once

#include <variant>
#include <vector>

#include "ast/AstNode.h"
#include "ast/Decl.h"
#include "ast/DeclContext.h"
#include "ast/Stmt.h"
#include "semantic/Semantic.h"
#include "utils/BumpAllocator.h"

namespace semantic {

class CFGNode {
   friend class CFGBuilder;
   std::pmr::vector<CFGNode*> children;
   std::pmr::vector<CFGNode*> parents;
   std::variant<const ast::Expr*, const ast::VarDecl*> data;
   CFGNode(std::variant<const ast::Expr*, const ast::VarDecl*> data) : data(data) {
      children = std::pmr::vector<CFGNode*>();
      parents = std::pmr::vector<CFGNode*>();
   }

public:
};

class CFGBuilder {
   using Heap = std::pmr::memory_resource;

private:
   CFGNode* buildIteratively(const ast::Stmt* stmt, CFGNode* parent = nullptr);
   CFGNode* buildForStmt(const ast::ForStmt* forStmt, CFGNode* parent = nullptr);
   CFGNode* buildIfStmt(const ast::IfStmt* ifStmt, CFGNode* parent = nullptr);
   CFGNode* buildDeclStmt(const ast::DeclStmt* declStmt,
                          CFGNode* parent = nullptr);
   CFGNode* buildExprStmt(const ast::ExprStmt* exprStmt,
                          CFGNode* parent = nullptr);
   CFGNode* buildReturnStmt(const ast::ReturnStmt* returnStmt,
                            CFGNode* parent = nullptr);
   CFGNode* buildWhileStmt(const ast::WhileStmt* whileStmt,
                           CFGNode* parent = nullptr);

public:
   CFGBuilder(Heap* heap, ast::Semantic& sema)
         : alloc{heap}, heap{heap}, sema(sema) {}
   CFGNode* build(const ast::Stmt* stmt) { return buildIteratively(stmt); }

private:
   mutable BumpAllocator alloc;
   Heap* heap;
   ast::Semantic& sema;
};
} // namespace semantic