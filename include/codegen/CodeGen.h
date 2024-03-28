#pragma once

#include "ast/AST.h"
#include "ast/DeclContext.h"
#include "ast/Stmt.h"
#include "semantic/NameResolver.h"
#include "tir/Constant.h"
#include "tir/IRBuilder.h"
#include "tir/Instructions.h"
#include "tir/TIR.h"

namespace codegen {

class CGExprEvaluator;

class CodeGenerator {
   friend class CGExprEvaluator;
public:
   CodeGenerator(tir::Context& ctx, tir::CompilationUnit& cu, semantic::NameResolver& nr)
         : ctx{ctx}, cu{cu}, nr{nr} {}
   void run(ast::LinkingUnit const* lu);
   tir::Type* emitType(ast::Type const* type);

private:
   void emitStmt(ast::Stmt const* stmt);
   tir::Value* emitExpr(ast::Expr const* expr);
   void emitFunction(ast::MethodDecl const* decl);
   void emitFunctionDecl(ast::MethodDecl const* decl);
   void emitClassDecl(ast::ClassDecl const* decl);
   void emitClass(ast::ClassDecl const* decl);

private:
   void emitReturnStmt(ast::ReturnStmt const* stmt);
   void emitForStmt(ast::ForStmt const* stmt);
   void emitBlockStmt(ast::BlockStatement const* stmt);
   void emitDeclStmt(ast::DeclStmt const* stmt);
   void emitExprStmt(ast::ExprStmt const* stmt);
   void emitIfStmt(ast::IfStmt const* stmt);
   void emitWhileStmt(ast::WhileStmt const* stmt);

private:
   tir::Context& ctx;
   tir::CompilationUnit& cu;
   tir::Function* curFn{nullptr};
   std::pmr::unordered_map<ast::Decl const*, tir::AllocaInst*> valueMap{ctx.alloc()};
   std::pmr::unordered_map<ast::Decl const*, tir::Value*> gvMap{ctx.alloc()};
   tir::IRBuilder builder{ctx};
   semantic::NameResolver& nr;
};

} // namespace codegen
