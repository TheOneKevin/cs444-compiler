#pragma once

#include "ast/AstNode.h"
#include "ast/Decl.h"
#include "ast/DeclContext.h"
#include "ast/Expr.h"
#include "ast/Stmt.h"
#include "ast/Type.h"

namespace ast {

static std::string indent(int indentation) {
    return std::string(indentation * 2, ' ');
}

} // namespace ast
