#include "ast/Stmt.h"
#include "codegen/CodeGen.h"
#include "tir/IRBuilder.h"

using namespace tir;
using CG = codegen::CodeGenerator;

/* ===--------------------------------------------------------------------=== */
// Emit specific statements
/* ===--------------------------------------------------------------------=== */

namespace codegen {

void CG::emitForStmt(ast::ForStmt const* stmt) { (void)stmt; }

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
   } else {
      assert(false && "Unknown statement type");
   }
}

} // namespace codegen
