#include "ast/Stmt.h"
#include "codegen/CodeGen.h"

using namespace tir;

/* ===--------------------------------------------------------------------=== */
// Emit specific statements
/* ===--------------------------------------------------------------------=== */

namespace {

Value* emitForStmt(ast::ForStmt* stmt) {
   (void)stmt;
   return nullptr;
}

} // namespace

/* ===--------------------------------------------------------------------=== */
// CodeGenerator emit router
/* ===--------------------------------------------------------------------=== */

namespace codegen {

Value* CodeGenerator::emitStmt(ast::Stmt* stmt) {
   if(auto* forStmt = cast<ast::ForStmt*>(stmt)) {
      return emitForStmt(forStmt);
   } else {
      assert(false && "Unknown statement type");
   }
}

} // namespace codegen
