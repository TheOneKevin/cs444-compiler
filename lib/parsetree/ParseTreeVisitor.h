#pragma once

#include "parsetree/ParseTreeTypes.h"
#include "ast/AST.h"

namespace parsetree {

ast::CompilationUnit* visitCompilationUnit(Node* node);
ast::PackageDeclaration* visitPackageDeclaration(Node* node);
ast::ImportDeclarations* visitImportDeclarations(Node* node);
ast::TypeDeclarations* visitTypeDeclarations(Node* node);

ast::QualifiedIdentifier* visitQualifiedIdentifier(Node *node, std::vector<Identifier*>& identifiers);

ast::Identifier* visitIdentifier(Node *node);

} // namespace parsetree

