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

/**
 * @brief Returns if the AST type will be mapped to a pointer in IR or not.
 *
 * @param ty The AST type
 * @return true True for strings, null and references.
 * @return false False for arrays and primitive types.
 */
static bool isAstTypeReference(ast::Type const* ty) {
   return ty->isString() || ty->isNull() || ty->isReference();
}

class CodeGenerator {
   friend class CGExprEvaluator;

public:
   CodeGenerator(tir::Context& ctx, tir::CompilationUnit& cu,
                 semantic::NameResolver& nr);
   CodeGenerator(CodeGenerator const&) = delete;
   CodeGenerator(CodeGenerator&&) = delete;
   CodeGenerator& operator=(CodeGenerator const&) = delete;
   CodeGenerator& operator=(CodeGenerator&&) = delete;
   void run(ast::LinkingUnit const* lu);
   tir::Type* emitType(ast::Type const* type);

private:
   void emitStmt(ast::Stmt const* stmt);
   tir::Value* emitExpr(ast::Expr const* expr);
   // Emit the function body, assuming the declaration is there
   void emitFunction(ast::MethodDecl const* decl);
   // Emit just the declaration, no body
   void emitFunctionDecl(ast::MethodDecl const* decl);
   // Emit the class type and static fields, no body
   void emitClassDecl(ast::ClassDecl const* decl);
   // Emit the class body (methods and field initializers)
   void emitClass(ast::ClassDecl const* decl);

private:
   // Given a pointer to the array struct, read out the size
   tir::Value* emitGetArraySz(tir::Value* ptr);
   // Given a pointer to the array struct, read out the pointer
   tir::Value* emitGetArrayPtr(tir::Value* ptr);
   // Given a pointer to the array struct, set the size
   void emitSetArraySz(tir::Value* ptr, tir::Value* sz);
   // Given a pointer to the array struct, set the pointer
   void emitSetArrayPtr(tir::Value* ptr, tir::Value* arr);

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
   // Local AST local decl -> IR Alloca
   std::unordered_map<ast::Decl const*, tir::AllocaInst*> valueMap{};
   // Global static AST func/field -> IR Global
   std::unordered_map<ast::Decl const*, tir::Value*> gvMap{};
   // Global AST Class -> IR Type
   std::unordered_map<ast::Decl const*, tir::Type*> typeMap{};
   // Array type (cache)
   tir::StructType* arrayType{nullptr};
   tir::IRBuilder builder{ctx};
   semantic::NameResolver& nr;
};

} // namespace codegen
