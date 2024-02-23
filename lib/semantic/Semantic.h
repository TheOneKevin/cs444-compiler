#pragma once

#include <unordered_set>

#include "ast/AST.h"
#include "diagnostics/Diagnostics.h"
#include "diagnostics/Location.h"
#include "parsetree/ParseTree.h"
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

   UnresolvedType* BuildUnresolvedType(SourceRange loc);
   ArrayType* BuildArrayType(Type* elementType, SourceRange loc);
   BuiltInType* BuildBuiltInType(parsetree::BasicType::Type type, SourceRange loc);

   /* ===-----------------------------------------------------------------=== */
   // ast/Decl.h
   /* ===-----------------------------------------------------------------=== */

   VarDecl* BuildVarDecl(Type* type, SourceRange location, string_view name,
                         Expr* init = nullptr);
   FieldDecl* BuildFieldDecl(Modifiers modifiers, SourceRange location, Type* type,
                             string_view name, Expr* init = nullptr);

   /* ===-----------------------------------------------------------------=== */
   // ast/DeclContext.h
   /* ===-----------------------------------------------------------------=== */

   LinkingUnit* BuildLinkingUnit(array_ref<CompilationUnit*> compilationUnits);
   CompilationUnit* BuildCompilationUnit(ReferenceType* package,
                                         array_ref<ImportDeclaration> imports,
                                         SourceRange location, DeclContext* body);
   ClassDecl* BuildClassDecl(Modifiers modifiers, SourceRange location,
                             string_view name, ReferenceType* superClass,
                             array_ref<ReferenceType*> interfaces,
                             array_ref<Decl*> classBodyDecls);
   InterfaceDecl* BuildInterfaceDecl(Modifiers modifiers, SourceRange location,
                                     string_view name,
                                     array_ref<ReferenceType*> extends,
                                     array_ref<Decl*> interfaceBodyDecls);
   MethodDecl* BuildMethodDecl(Modifiers modifiers, SourceRange location,
                               string_view name, Type* returnType,
                               array_ref<VarDecl*> parameters, bool isConstructor,
                               Stmt* body);
   /* ===-----------------------------------------------------------------=== */
   // ast/Stmt.h
   /* ===-----------------------------------------------------------------=== */

   BlockStatement* BuildBlockStatement(array_ref<Stmt*> stmts);
   DeclStmt* BuildDeclStmt(VarDecl* decl);
   ExprStmt* BuildExprStmt(Expr* expr);
   IfStmt* BuildIfStmt(Expr* condition, Stmt* thenStmt, Stmt* elseStmt = nullptr);
   WhileStmt* BuildWhileStmt(Expr* condition, Stmt* body);
   ForStmt* BuildForStmt(Stmt* init, Expr* condition, Stmt* update, Stmt* body);
   ReturnStmt* BuildReturnStmt(Expr* expr);
   NullStmt* BuildNullStmt() { return alloc.new_object<NullStmt>(); }

public:
   BumpAllocator& allocator() { return alloc; }

public:
   /// @brief Clears the lexical local scope names
   void ResetLexicalLocalScope() {
      lexicalLocalScope.clear();
      lexicalLocalDecls.clear();
      lexicalLocalDeclStack.clear();
   }

   /**
    * @brief Checks if a name is in the lexical local scope
    * and if it is not, add it to the scope.
    * @param name The name to add to the scope
    * @return true If the name was added to the scope
    * @return false If the name was already in the scope
    */
   bool AddLexicalLocal(VarDecl* decl) {
      std::string nameCpy{decl->name()};
      if(lexicalLocalScope.find(nameCpy) != lexicalLocalScope.end()) return false;
      lexicalLocalScope.insert(std::move(nameCpy));
      lexicalLocalDecls.push_back(decl);
      lexicalLocalDeclStack.push_back(decl);
      return true;
   }

   /**
    * @brief Called when a new lexical scope is entered. Returns the size of the
    * local declaration stack so it can be restored when the scope is exited.
    * @return int Returns the size of the local declaration stack
    */
   int EnterLexicalScope() {
      int size = lexicalLocalDeclStack.size();
      return size;
   }

   /**
    * @brief Called when a lexical scope is exited. Resizes the local decl
    * stack to the size it was when the scope was entered. Deletes the locals
    * from the set as well.
    * @param size The size of the local declaration stack when the scope was
    * entered
    */
   void ExitLexicalScope(int size) {
      for(int i = lexicalLocalDeclStack.size() - 1; i >= size; --i)
         lexicalLocalScope.erase(lexicalLocalDeclStack[i]->name().data());
      lexicalLocalDeclStack.resize(size);
   }

   auto getAllLexicalDecls() const { return std::views::all(lexicalLocalDecls); }

private:
   BumpAllocator& alloc;
   diagnostics::DiagnosticEngine& diag;
   std::vector<VarDecl*> lexicalLocalDeclStack;
   std::vector<VarDecl*> lexicalLocalDecls;
   std::unordered_set<std::string> lexicalLocalScope;
};

} // namespace ast
