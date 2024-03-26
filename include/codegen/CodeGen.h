#pragma once

#include "ast/AST.h"
#include "tir/TIR.h"

namespace codegen {

class CodeGenerator {
public:
   CodeGenerator(tir::Context& ctx, tir::CompilationUnit& cu) : ctx{ctx}, cu{cu} {}

private:
   tir::Value* emitStmt(ast::Stmt* stmt);
   tir::Value* emitExpr(ast::Expr* expr);
   tir::Value* emitFunction(ast::Decl* decl);

private:
   tir::Context& ctx;
   tir::CompilationUnit& cu;
};

} // namespace codegen
