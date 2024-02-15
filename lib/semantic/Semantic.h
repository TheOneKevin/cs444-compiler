#pragma once

#include "ast/AST.h"
#include "diagnostics/Diagnostics.h"
#include "utils/BumpAllocator.h"

namespace ast {
class Semantic {
   using string = std::string;

public:
   Semantic(BumpAllocator& alloc, diagnostics::DiagnosticEngine& diag)
         : alloc{alloc}, diag{diag} {}

public:
   /* ===-----------------------------------------------------------------=== */
   // ast/Type.h
   /* ===-----------------------------------------------------------------=== */

   UnresolvedType* BuildUnresolvedType();

   /* ===-----------------------------------------------------------------=== */
   // ast/Decl.h
   /* ===-----------------------------------------------------------------=== */

   VarDecl* BuildVarDecl(Type* type, string_view name, Expr* init = nullptr);
   FieldDecl* BuildFieldDecl(Modifiers modifiers,
                             Type* type,
                             string_view name,
                             Expr* init = nullptr);

   /* ===-----------------------------------------------------------------=== */
   // ast/DeclContext.h
   /* ===-----------------------------------------------------------------=== */

   CompilationUnit* BuildCompilationUnit(ReferenceType* package,
                                         array_ref<ImportDeclaration> imports,
                                         DeclContext* body);

   ClassDecl* BuildClassDecl(Modifiers modifiers,
                             string_view name,
                             ReferenceType* superClass,
                             array_ref<ReferenceType*> interfaces,
                             array_ref<Decl*> classBodyDecls);

   InterfaceDecl* BuildInterfaceDecl(Modifiers modifiers,
                                     string_view name,
                                     array_ref<ReferenceType*> extends,
                                     array_ref<Decl*> interfaceBodyDecls);

   MethodDecl* BuildMethodDecl(Modifiers modifiers,
                               string_view name,
                               Type* returnType,
                               array_ref<VarDecl*> parameters,
                               bool isConstructor,
                               Stmt* body);
   /* ===-----------------------------------------------------------------=== */
   // ast/Stmt.h
   /* ===-----------------------------------------------------------------=== */

   BlockStatement* BuildBlockStatement(array_ref<Stmt*> stmts);
   DeclStmt* BuildDeclStmt(VarDecl* decl);
   ExprStmt* BuildExprStmt(Expr* expr);
   IfStmt* BuildIfStmt(Expr* condition,
                       Stmt* thenStmt,
                       Stmt* elseStmt = nullptr);
   WhileStmt* BuildWhileStmt(Expr* condition, Stmt* body);
   ForStmt* BuildForStmt(Stmt* init, Expr* condition, Stmt* update, Stmt* body);
   ReturnStmt* BuildReturnStmt(Expr* expr);
   NullStmt* BuildNullStmt() { return alloc.new_object<NullStmt>(); }

   BumpAllocator& getAllocator() { return alloc; }

private:
   BumpAllocator& alloc;
   diagnostics::DiagnosticEngine& diag;
};

} // namespace ast
