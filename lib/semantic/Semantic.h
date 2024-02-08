#pragma once

#include "ast/AST.h"

namespace ast {

class Semantic {
    using string = std::string;
public:
    /* ===----------------------------------------------------------------=== */
    // ast/Decl.h
    /* ===----------------------------------------------------------------=== */

    VarDecl* BuildVarDecl(Type* type, string name);
    FieldDecl* BuildFieldDecl(Modifiers modifiers, Type* type, string name);

    /* ===----------------------------------------------------------------=== */
    // ast/DeclContext.h
    /* ===----------------------------------------------------------------=== */

    CompilationUnit* BuildCompilationUnit(QualifiedIdentifier* package,
                                         std::vector<ImportDeclaration> imports,
                                         DeclContext* body);

    

};

} // namespace ast
