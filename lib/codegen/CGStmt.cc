#include "ast/Stmt.h"
#include "codegen/CodeGen.h"
#include "tir/IRBuilder.h"

using namespace tir;
using CG = codegen::CodeGenerator;

/* ===--------------------------------------------------------------------=== */
// Emit specific statements
/* ===--------------------------------------------------------------------=== */

namespace codegen {

void CG::emitForStmt(ast::ForStmt const* stmt) { 
   if (stmt->init()) {
      emitStmt(stmt->init());
   }
   auto cond = builder.createBasicBlock(curFn);
   auto body = builder.createBasicBlock(curFn);
   auto afterFor = builder.createBasicBlock(curFn);
   builder.createBranchInstr(cond);
   builder.setInsertPoint(cond->begin());
   if (stmt->condition()) {
      auto* condVal = emitExpr(stmt->condition());
      builder.createBranchInstr(condVal, body, afterFor);
   } else {
      builder.createBranchInstr(body); // no condition, always branches to body
   }
   builder.setInsertPoint(body->begin());
   emitStmt(stmt->body());
   if (stmt->update()) {
      emitStmt(stmt->update());
   }
   builder.createBranchInstr(cond);
   builder.setInsertPoint(afterFor->begin());
}

void CG::emitReturnStmt(ast::ReturnStmt const* stmt) {
   if(auto expr = stmt->expr()) {
      auto* val = emitExpr(expr);
      builder.createReturnInstr(val);
   } else {
      builder.createReturnInstr();
   }
}

void CG::emitBlockStmt(ast::BlockStatement const* stmt) {
   for(auto* s : stmt->stmts()) {
      emitStmt(s);
   }
}

void CG::emitDeclStmt(ast::DeclStmt const* stmt) {
   auto decl = stmt->decl();
   // Grab the stack slot for the declaration (i.e., alloca inst)
   assert(valueMap.find(decl) != valueMap.end() &&
          "Declaration not found in the value map");
   auto alloca = valueMap[decl];
   // If there's a declaration, store the value into the stack slot
   if(auto* init = decl->init()) {
      auto* val = emitExpr(init);
      builder.createStoreInstr(val, alloca);
   }
}

void CG::emitExprStmt(ast::ExprStmt const* stmt) {
   emitExpr(stmt->expr());
}

void CG::emitIfStmt(ast::IfStmt const* stmt) {
   auto cond = emitExpr(stmt->condition());
   auto thenBB = builder.createBasicBlock(curFn);
   auto elseBB = builder.createBasicBlock(curFn);
   auto afterIf = builder.createBasicBlock(curFn);
   builder.createBranchInstr(cond, thenBB, elseBB);

   // build the then block
   builder.setInsertPoint(thenBB->begin());
   emitStmt(stmt->thenStmt());
   builder.createBranchInstr(afterIf);

   // build the else block
   builder.setInsertPoint(elseBB->begin());
   if (stmt->elseStmt()) {
      emitStmt(stmt->elseStmt());
   }
   builder.createBranchInstr(afterIf);

   // set the insert point to the afterIf block
   builder.setInsertPoint(afterIf->begin());
}

void CG::emitWhileStmt(ast::WhileStmt const* stmt) {
   auto cond = builder.createBasicBlock(curFn);
   auto body = builder.createBasicBlock(curFn);
   auto afterWhile = builder.createBasicBlock(curFn);
   builder.createBranchInstr(cond);

   // build the condition block
   builder.setInsertPoint(cond->begin());
   auto* condVal = emitExpr(stmt->condition());
   builder.createBranchInstr(condVal, body, afterWhile);

   // build the body block
   builder.setInsertPoint(body->begin());
   emitStmt(stmt->body());
   builder.createBranchInstr(cond);

   // set the insert point to the afterWhile block
   builder.setInsertPoint(afterWhile->begin());
}



} // namespace codegen

/* ===--------------------------------------------------------------------=== */
// CodeGenerator emit router
/* ===--------------------------------------------------------------------=== */

namespace codegen {

void CG::emitStmt(ast::Stmt const* stmt) {
   if(auto* forStmt = dyn_cast<ast::ForStmt>(stmt)) {
      emitForStmt(forStmt);
   } else if(auto* retStmt = dyn_cast<ast::ReturnStmt>(stmt)) {
      emitReturnStmt(retStmt);
   } else if(auto* blockStmt = dyn_cast<ast::BlockStatement>(stmt)) {
      emitBlockStmt(blockStmt);
   } else if(auto* declStmt = dyn_cast<ast::DeclStmt>(stmt)) {
      emitDeclStmt(declStmt);
   } else if (auto* exprStmt = dyn_cast<ast::ExprStmt>(stmt)) {
      emitExprStmt(exprStmt);
   } else if (auto* ifStmt = dyn_cast<ast::IfStmt>(stmt)) {
      emitIfStmt(ifStmt);
   } else if (auto* whileStmt = dyn_cast<ast::WhileStmt>(stmt)) {
      emitWhileStmt(whileStmt);
   } else if (auto* nullStmt = dyn_cast<ast::NullStmt>(stmt)) {
      // Do nothing
   } else {
      assert(false && "Unknown statement type");
   }
}

} // namespace codegen
