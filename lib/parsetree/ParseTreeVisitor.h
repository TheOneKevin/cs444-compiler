#pragma once

#include "parsetree/ParseTreeTypes.h"
#include "ast/AST.h"

namespace parsetree {

ast::CompilationUnit* visitCompilationUnit(Node* node);
ast::PackageDeclaration* visitPackageDeclaration(Node* node);
ast::ImportDeclarations* visitImportDeclarations(Node* node, std::vector<ast::Import *>& imports);
ast::Import* visitImport(Node *node);
ast::TypeDeclarations* visitTypeDeclarations(Node* node);

ast::QualifiedIdentifier* visitQualifiedIdentifier(Node *node, std::vector<ast::Identifier*>& identifiers);

ast::Identifier* visitIdentifier(Node *node);

} // namespace parsetree

