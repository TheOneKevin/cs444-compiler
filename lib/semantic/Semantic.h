#pragma once

#include "ast/AST.h"
#include "utils/BumpAllocator.h"

namespace ast {

class Semantic {
   using string = std::string;

public:
   Semantic(BumpAllocator& alloc) : alloc(alloc) {}

public:
   /* ===-----------------------------------------------------------------=== */
   // ast/Decl.h
   /* ===-----------------------------------------------------------------=== */

   VarDecl* BuildVarDecl(Type* type, string_view name);
   FieldDecl* BuildFieldDecl(Modifiers modifiers, Type* type, string_view name);

   /* ===-----------------------------------------------------------------=== */
   // ast/DeclContext.h
   /* ===-----------------------------------------------------------------=== */

   CompilationUnit* BuildCompilationUnit(QualifiedIdentifier* package,
                                         array_ref<ImportDeclaration> imports,
                                         DeclContext* body);

   ClassDecl* BuildClassDecl(Modifiers modifiers,
                             string_view name,
                             QualifiedIdentifier* superClass,
                             array_ref<QualifiedIdentifier*> interfaces,
                             array_ref<Decl*> classBodyDecls);

   InterfaceDecl* BuildInterfaceDecl(Modifiers modifiers,
                                     string_view name,
                                     array_ref<QualifiedIdentifier*> extends,
                                     array_ref<Decl*> interfaceBodyDecls);

   MethodDecl* BuildMethodDecl(Modifiers modifiers,
                               string_view name,
                               Type* returnType,
                               array_ref<VarDecl*> parameters,
                               bool isConstructor,
                               Stmt* body);

private:
   BumpAllocator& alloc;
};

} // namespace ast
