#include "ParseTreeTypes.h"
#include "ParseTreeVisitor.h"

using namespace parsetree;

ast::CompilationUnit* parsetree::visitCompilationUnit(Node* node) {
    // Check the node we're visiting
    if (node->get_type() != Node::Type::CompilationUnit) {
        throw std::runtime_error("VisitCompilationUnit called on a node that is not a CompilationUnit");
    }
    // Type check the children nodes
    if (node->num_children() != 3) {
        throw std::runtime_error("CompilationUnit node must have 3 children");
    }
    return new ast::CompilationUnit {
        visitPackageDeclaration(node->child(0)),
        visitImportDeclarations(node->child(1)),
        visitTypeDeclarations(node->child(2))
    };
}

ast::PackageDeclaration* parsetree::visitPackageDeclaration(Node* node) {
    if (node == nullptr) return nullptr; // nullable
    if (node->get_type() != Node::Type::PackageDeclaration) {
        throw std::runtime_error("PackageDeclaration called on a node that is not a PackageDeclaration");
    }
    // Type check the children nodes
    if (node->num_children() != 1) {
        throw std::runtime_error("PackageDeclaration node must have 1 children");
    }
    return new ast::PackageDeclaration {
        visitQualifiedIdentifier(node->child(0), *(new std::vector<ast::Identifier*>()))
    };
}

ast::ImportDeclarations* parsetree::visitImportDeclarations(Node* node) {
    
}

ast::TypeDeclarations* parsetree::visitTypeDeclarations(Node* node) {

}

ast::QualifiedIdentifier* parsetree::visitQualifiedIdentifier(Node *node, std::vector<ast::Identifier* >& identifiers) {
    if (node->get_type() != Node::Type::QualifiedIdentifier) {
        throw std::runtime_error("QualifiedIdentifier called on a node that is not a QualifiedIdentifier");
    }
    if (node->child(0)->get_type() == Node::Type::Identifier) {
        identifiers.push_back(visitIdentifier(node->child(0)));
    } else if (node->child(0)->get_type() == Node::Type::QualifiedIdentifier){
        visitQualifiedIdentifier(node->child(0), identifiers);
        identifiers.push_back(visitIdentifier(node->child(1)));
    } else {
        throw std::runtime_error("QualifiedIdentifier node has incorrect children nodes");
    }

    return new ast::QualifiedIdentifier(identifiers);
}

ast::Identifier* parsetree::visitIdentifier(Node *node) {
    if (node->get_type() != Node::Type::Identifier) {
        throw std::runtime_error("Identifier called on a node that is not a Identifier");
    }
    return new ast::Identifier(
        dynamic_cast<Identifier*>(node)->get_name()
    );
}