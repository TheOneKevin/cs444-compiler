#pragma once

#include "parsetree/ParseTreeTypes.h"
#include "ast/AST.h"

namespace parsetree {

ast::CompilationUnit* visitCompilationUnit(Node* node);
void visitPackageDeclaration(Node* node, ast::QualifiedIdentifier& ast_node);
void visitImportDeclarations(Node* node, std::vector<ast::ImportDeclaration>& imports);
ast::ImportDeclaration visitImport(Node *node);
ast::DeclContext* visitTypeDeclaration(Node* node);

// Classes & interfaces
ast::ClassDeclaration* visitClassDeclaration(Node* node);
ast::InterfaceDeclaration* visitInterfaceDeclaration(Node* node);
void visitClassOrInterfaceModifiers(Node* node, std::vector<ast::ClassOrInterfaceModifier*>& modifiers);
void visitInterfaceTypeList(Node* node, std::vector<ast::QualifiedIdentifier*>& interfaces);
void visitClassBodyDeclarations(Node* node, std::vector<ast::ClassBodyDeclaration*>& declarations);
ast::QualifiedIdentifier* visitSuperOpt(Node* node);
ast::ClassOrInterfaceModifier* visitClassOrInterfaceModifier(Node* node);
void visitInterfaceTypeList(Node* node, std::vector<ast::QualifiedIdentifier*>& interfaces);

void visitQualifiedIdentifier(Node *node, ast::QualifiedIdentifier& ast_node);

std::string visitIdentifier(Node *node);

} // namespace parsetree

