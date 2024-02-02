#include "ParseTreeTypes.h"
#include "ParseTreeVisitor.h"

namespace parsetree {

using pty = Node::Type;

ast::CompilationUnit* visitCompilationUnit(Node* node) {
    // Check the node we're visiting
    check_node_type(node, pty::CompilationUnit);
    check_num_children(node, 3, 3);
    // $1: Visit the package declaration
    auto package = visitPackageDeclaration(node->child(0));
    // $2: Visit the import declarations
    std::vector<ast::ImportDeclaration> imports;
    visitListPattern<
        pty::ImportDeclarationList, ast::ImportDeclaration, true
    >(node->child(1), imports);
    // $3: Visit the body, if it is not null
    ast::DeclContext* body_ast_node = nullptr;
    if (auto body = node->child(2)) {
        if (body->get_type() == pty::ClassDeclaration) {
            body_ast_node = visitClassDeclaration(body);
        } else if (body->get_type() == pty::InterfaceDeclaration) {
            body_ast_node = visitInterfaceDeclaration(body);
        }
    }
    // Return the constructed AST node
    return new ast::CompilationUnit { package, imports, body_ast_node };
}

ast::QualifiedIdentifier* visitPackageDeclaration(Node* node) {
    if (node == nullptr) return nullptr;
    check_node_type(node, pty::PackageDeclaration);
    check_num_children(node, 1, 1);
    return visitQualifiedIdentifier(node->child(0));
}

template<>
ast::ImportDeclaration visit<pty::ImportDeclarationList>(Node* node) {
    check_num_children(node, 1, 1);
    auto id = visitQualifiedIdentifier(node->child(0));
    if (node->get_type() == pty::SingleTypeImportDeclaration) {
        return ast::ImportDeclaration{id, false};
    } else if (node->get_type() == pty::TypeImportOnDemandDeclaration) {
        return ast::ImportDeclaration{id, true};
    }
    throw std::runtime_error("Import called on a node that is not a Import");
}

} // namespace parsetree
