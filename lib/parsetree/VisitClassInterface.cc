#include "ParseTreeTypes.h"
#include "ParseTreeVisitor.h"

using namespace parsetree;

ast::ClassDeclaration* parsetree::visitClassDeclaration(Node* node) {
    if (node->get_type() != Node::Type::ClassDeclaration) {
        throw std::runtime_error("VisitClassDeclaration called on a node that is not a ClassDeclaration");
    }
    if (node->num_children() != 5) {
        throw std::runtime_error("ClassDeclaration node must have 5 children");
    }
    // get the modifiers
    std::vector<ast::ClassOrInterfaceModifier*> modifiers;
    visitClassOrInterfaceModifiers(node->child(0), modifiers);

    // get the interface type list
    std::vector<ast::QualifiedIdentifier*> interfaces;
    visitInterfaceTypeList(node->child(3), interfaces);

    // get the class body declarations
    std::vector<ast::ClassBodyDeclaration*> classBodyDeclarations;
    visitClassBodyDeclarations(node->child(4), classBodyDeclarations);
    return new ast::ClassDeclaration {
        modifiers,
         visitIdentifier(node->child(1)),
        visitSuperOpt(node->child(2)),
        interfaces,
        classBodyDeclarations
    };
}

void parsetree::visitClassOrInterfaceModifiers(Node* node, std::vector<ast::ClassOrInterfaceModifier*>& modifiers) {
    if (node == nullptr) return; // nullable
    if (node->child(0)->get_type() == Node::Type::ClassOrInterfaceModifierList) {
        visitClassOrInterfaceModifiers(node->child(0), modifiers);
        modifiers.push_back(visitClassOrInterfaceModifier(node->child(1)));
    } else {
        modifiers.push_back(visitClassOrInterfaceModifier(node->child(0)));
    }
}

ast::ClassOrInterfaceModifier* parsetree::visitClassOrInterfaceModifier(Node* node) {
    if (node->get_type() != Node::Type::Modifier) {
        throw std::runtime_error("visitClassOrInterfaceModifier called on a node that is not a Modifier");
    }
    Modifier::Type type = dynamic_cast<Modifier*>(node)->get_type();
    if (type == Modifier::Type::Abstract) {
        return new ast::ClassOrInterfaceModifier(ast::ClassOrInterfaceModifier::ModifierType::ABSTRACT);
    } else if (type == Modifier::Type::Final) {
        return new ast::ClassOrInterfaceModifier(ast::ClassOrInterfaceModifier::ModifierType::FINAL);
    } else if (type == Modifier::Type::Public) {
        return new ast::ClassOrInterfaceModifier(ast::ClassOrInterfaceModifier::ModifierType::PUBLIC);
    } else {
        throw std::runtime_error("Incorrect Modifier type for ClassOrInterfaceModifier");
    }
}

ast::QualifiedIdentifier* parsetree::visitSuperOpt(Node* node) {
    if (node == nullptr) return nullptr; // nullable
    if (node->get_type() != Node::Type::SuperOpt) {
        throw std::runtime_error("visitSuperOpt called on a node that is not a SuperOpt");
    }
    if (node->num_children() != 1) {
        throw std::runtime_error("SuperOpt node must have 1 children");
    }
    return visitQualifiedIdentifier(node->child(0), *(new std::vector<ast::Identifier*>()));

}

void parsetree::visitInterfaceTypeList(Node* node, std::vector<ast::QualifiedIdentifier*>& interfaces) {
    if (node == nullptr) return; // nullable
    if (node->child(0)->get_type() == Node::Type::InterfaceTypeList) {
        visitInterfaceTypeList(node->child(0), interfaces);
        interfaces.push_back(visitQualifiedIdentifier(node->child(1), *(new std::vector<ast::Identifier*>())));
    } else {
        interfaces.push_back(visitQualifiedIdentifier(node->child(0), *(new std::vector<ast::Identifier*>())));
    }
}

void parsetree::visitClassBodyDeclarations(Node* node, std::vector<ast::ClassBodyDeclaration*>& declarations) {

}

ast::InterfaceDeclaration* parsetree::visitInterfaceDeclaration(Node* node) {
        
}