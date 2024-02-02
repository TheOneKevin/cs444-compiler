#pragma once

#include "parsetree/ParseTreeTypes.h"
#include "ast/AST.h"

namespace parsetree {

using pty = Node::Type;

// Basic helper functions //////////////////////////////////////////////////////

static inline void check_node_type(Node* node, Node::Type type) {
    if (node->get_type() != type) {
        throw std::runtime_error(
            "Called on a node that is not the correct type!"
            " Expected: " + Node::type_string(type) +
            " Actual: " + node->type_string()
        );
    }
}

static inline void check_num_children(Node* node, int min, int max) {
    if (node->num_children() < min || node->num_children() > max) {
        throw std::runtime_error(
            "Node has incorrect number of children!"
            " Type: " + node->type_string() +
            " Expected: " + std::to_string(min) + " to " + std::to_string(max) +
            " Actual: " + std::to_string(node->num_children())
        );
    }
}

[[noreturn]] static inline void unreachable() {
    throw std::runtime_error("Unreachable code reached!");
}

// Templated visitor patterns //////////////////////////////////////////////////

// NOTE(kevin): Technically we can re-implement all our visitors using the
// template patterns. However, the syntax is ugly at best. We only use these
// for the list patterns, which are more tedious and benefit from templates.

template<parsetree::Node::Type N, typename T>
T visit(Node* node) {
    throw std::runtime_error("No visitor for node type " + node->type_string());
}

template<parsetree::Node::Type N, typename T, bool nullable = false>
void visitListPattern(Node* node, std::vector<T>& list) {
    if(nullable && node == nullptr) return;
    if(!nullable && node == nullptr)
        throw std::runtime_error("Visited a null node!");
    check_node_type(node, N);
    check_num_children(node, 1, 2);
    if(node->num_children() == 1) {
        list.push_back(visit<N, T>(node->child(0)));
    } else if(node->num_children() == 2) {
        visitListPattern<N, T, nullable>(node->child(0), list);
        list.push_back(visit<N, T>(node->child(1)));
    }
}

// Compilation unit visitors ///////////////////////////////////////////////////

ast::CompilationUnit* visitCompilationUnit(Node* node);
ast::QualifiedIdentifier* visitPackageDeclaration(Node* node);
template<> ast::ImportDeclaration visit<pty::ImportDeclarationList>(Node* node);

// Classes & interfaces visitors ///////////////////////////////////////////////

ast::ClassDecl* visitClassDeclaration(Node* node);
ast::InterfaceDecl* visitInterfaceDeclaration(Node* node);
ast::QualifiedIdentifier* visitSuperOpt(Node* node);
ast::FieldDecl* visitFieldDeclaration(Node* node);
ast::MethodDecl* visitMethodDeclaration(Node* node);
ast::MethodDecl* visitConstructorDeclaration(Node* node);

template<> ast::Decl* visit<pty::ClassBodyDeclarationList>(Node* node);
template<> ast::VarDecl* visit<pty::VariableDeclaratorList>(Node* node);
template<> ast::VarDecl* visit<pty::FormalParameterList>(Node* node);

// Statements visitors /////////////////////////////////////////////////////////

ast::Stmt* visitBlock(Node* node);

// Leaf node visitors //////////////////////////////////////////////////////////

ast::QualifiedIdentifier* visitQualifiedIdentifier(
    Node *node, ast::QualifiedIdentifier* ast_node = nullptr);
std::string visitIdentifier(Node *node);
ast::Modifiers visitModifierList(
    Node* node, ast::Modifiers modifiers = ast::Modifiers{});
Modifier visitModifier(Node* node);
ast::Type* visitType(Node* node);

} // namespace parsetree

