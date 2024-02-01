#pragma once

#include "parsetree/ParseTreeTypes.h"
#include "ast/AST.h"

namespace parsetree {

ast::CompilationUnit* VisitCompilationUnit(Node* node);

} // namespace parsetree

