#pragma once

#include "ast/AST.h"
#include "ast/Stmt.h"
#include "tir/Constant.h"
#include "tir/IRBuilder.h"
#include "tir/Instructions.h"
#include "tir/TIR.h"

namespace codegen {

class CodeGenerator {
public:
   CodeGenerator(tir::Context& ctx, tir::CompilationUnit& cu)
         : ctx{ctx}, cu{cu} {}

private:
   void emitStmt(ast::Stmt const* stmt);
   tir::Value* emitExpr(ast::Expr const* expr);
   void emitFunction(ast::MethodDecl const* decl);
   tir::Type* emitType(ast::Type const* type);

private:
   void emitReturnStmt(ast::ReturnStmt const* stmt);
   void emitForStmt(ast::ForStmt const* stmt);
   void emitBlockStmt(ast::BlockStatement const* stmt);
   void emitDeclStmt(ast::DeclStmt const* stmt);

private:
   tir::Context& ctx;
   tir::CompilationUnit& cu;
   tir::Function* curFn{nullptr};
   std::pmr::unordered_map<ast::Decl const*, tir::AllocaInst*> valueMap{ctx.alloc()};
   tir::IRBuilder builder{ctx};
};

} // namespace codegen
