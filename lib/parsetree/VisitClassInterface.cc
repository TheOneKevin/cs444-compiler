#include "ast/AST.h"
#include "parsetree/ParseTreeTypes.h"
#include "parsetree/ParseTreeVisitor.h"

using namespace parsetree;

ast::ClassDecl* parsetree::visitClassDeclaration(Node* node) {
    if (node->get_type() != Node::Type::ClassDeclaration) {
        throw std::runtime_error("VisitClassDeclaration called on a node that is not a ClassDeclaration");
    }
    if (node->num_children() != 5) {
        throw std::runtime_error("ClassDeclaration node must have 5 children");
    }
    // get the interface type list
    std::vector<ast::QualifiedIdentifier*> interfaces;
    visitInterfaceTypeList(node->child(3), interfaces);

    // get the class body declarations
    std::vector<ast::DeclContext*> classBodyDeclarations;
    visitClassBodyDeclarations(node->child(4), classBodyDeclarations);
    return new ast::ClassDecl {
        visitClassOrInterfaceModifiers(node->child(0), ast::Modifiers{}),
        visitIdentifier(node->child(1)),
        visitSuperOpt(node->child(2)),
        interfaces,
        classBodyDeclarations
    };
}

ast::Modifiers parsetree::visitClassOrInterfaceModifiers(Node* node, ast::Modifiers modifiers) {
    if (node == nullptr) return modifiers; // nullable
    if (node->child(0)->get_type() == Node::Type::ModifierList) {
        visitClassOrInterfaceModifiers(node->child(0), modifiers);
        modifiers.set(visitClassOrInterfaceModifier(node->child(1)));
    } else {
        modifiers.set(visitClassOrInterfaceModifier(node->child(0)));
    }
}

ast::Modifiers parsetree::visitClassOrInterfaceModifier(Node* node) {
    if (node->get_type() != Node::Type::Modifier) {
        throw std::runtime_error("visitClassOrInterfaceModifier called on a node that is not a Modifier");
    }
    Modifier::Type type = dynamic_cast<Modifier*>(node)->get_type();
    if (!(type == Modifier::Type::Abstract || type == Modifier::Type::Final || type == Modifier::Type::Public)) {
        throw std::runtime_error("Incorrect Modifier type for ClassOrInterfaceModifier");
    }
    auto modifiers = ast::Modifiers{};
    modifiers.set(type);
    return modifiers;
}

ast::QualifiedIdentifier* parsetree::visitSuperOpt(Node* node) {
    if (node == nullptr) return nullptr; // nullable
    if (node->get_type() != Node::Type::SuperOpt) {
        throw std::runtime_error("visitSuperOpt called on a node that is not a SuperOpt");
    }
    if (node->num_children() != 1) {
        throw std::runtime_error("SuperOpt node must have 1 children");
    }
    return visitQualifiedIdentifier(node->child(0), new ast::QualifiedIdentifier{});

}

void parsetree::visitInterfaceTypeList(Node* node, std::vector<ast::QualifiedIdentifier*>& interfaces) {
    if (node == nullptr) return; // nullable
    auto id_node = new ast::QualifiedIdentifier{};
    if (node->child(0)->get_type() == Node::Type::InterfaceTypeList) {
        visitInterfaceTypeList(node->child(0), interfaces);
        visitQualifiedIdentifier(node->child(1), id_node);
    } else {
        visitQualifiedIdentifier(node->child(0), id_node);
    }
    interfaces.push_back(id_node);
}

void parsetree::visitClassBodyDeclarations(Node* node, std::vector<ast::DeclContext*>& declarations) {

}

ast::InterfaceDecl* parsetree::visitInterfaceDeclaration(Node* node) {

}