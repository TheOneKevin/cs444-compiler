#pragma once

#include "parsetree/ParseTreeTypes.h"
#include "ast/AST.h"

namespace parsetree {

ast::CompilationUnit* visitCompilationUnit(Node* node);
ast::QualifiedIdentifier* visitPackageDeclaration(Node* node);
void visitImportDeclarations(Node* node, std::vector<ast::ImportDeclaration>& imports);
ast::ImportDeclaration visitImport(Node *node);
ast::DeclContext* visitTypeDeclaration(Node* node);

// Classes & interfaces
ast::ClassDecl* visitClassDeclaration(Node* node);
ast::InterfaceDecl* visitInterfaceDeclaration(Node* node);
ast::Modifiers visitClassOrInterfaceModifiers(Node* node, ast::Modifiers modifiers);
ast::Modifiers visitClassOrInterfaceModifier(Node* node);
void visitInterfaceTypeList(Node* node, std::vector<ast::QualifiedIdentifier*>& interfaces);
void visitClassBodyDeclarations(Node* node, std::vector<ast::DeclContext*>& declarations);
ast::QualifiedIdentifier* visitSuperOpt(Node* node);
void visitInterfaceTypeList(Node* node, std::vector<ast::QualifiedIdentifier*>& interfaces);

ast::QualifiedIdentifier* visitQualifiedIdentifier(Node *node, ast::QualifiedIdentifier* ast_node);

std::string visitIdentifier(Node *node);

} // namespace parsetree

