#pragma once

#include "parsetree/ParseTreeTypes.h"
#include "ast/AST.h"

namespace parsetree {

ast::CompilationUnit* visitCompilationUnit(Node* node);
ast::QualifiedIdentifier* visitPackageDeclaration(Node* node);
void visitImportDeclarations(Node* node, std::vector<ast::Import *>& imports);
ast::Import* visitImport(Node *node);
ast::TypeDeclaration* visitTypeDeclaration(Node* node);

// Classes & interfaces
ast::ClassDeclaration* visitClassDeclaration(Node* node);
ast::InterfaceDeclaration* visitInterfaceDeclaration(Node* node);
void visitClassOrInterfaceModifiers(Node* node, std::vector<ast::ClassOrInterfaceModifier*>& modifiers);
void visitInterfaceTypeList(Node* node, std::vector<ast::QualifiedIdentifier*>& interfaces);
void visitClassBodyDeclarations(Node* node, std::vector<ast::ClassBodyDeclaration*>& declarations);
ast::QualifiedIdentifier* visitSuperOpt(Node* node);
ast::ClassOrInterfaceModifier* visitClassOrInterfaceModifier(Node* node);
void visitInterfaceTypeList(Node* node, std::vector<ast::QualifiedIdentifier*>& interfaces);

ast::QualifiedIdentifier* visitQualifiedIdentifier(Node *node, std::vector<ast::Identifier*>& identifiers);

ast::Identifier* visitIdentifier(Node *node);

} // namespace parsetree

