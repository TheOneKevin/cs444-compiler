#include "ParseTreeTypes.h"
#include "ParseTreeVisitor.h"

namespace parsetree {

using pty = Node::Type;

ast::QualifiedIdentifier* visitQualifiedIdentifier(Node *node, ast::QualifiedIdentifier* ast_node) {
    check_node_type(node, pty::QualifiedIdentifier);
    check_num_children(node, 1, 2);
    if(ast_node == nullptr) {
        ast_node = new ast::QualifiedIdentifier{};
    }
    if(node->num_children() == 1) {
        ast_node->addIdentifier(visitIdentifier(node->child(0)));
        return ast_node;
    } else if(node->num_children() == 2) {
        ast_node = visitQualifiedIdentifier(node->child(0), ast_node);
        ast_node->addIdentifier(visitIdentifier(node->child(1)));
        return ast_node;
    }
    unreachable();
}

std::string visitIdentifier(Node *node) {
    check_node_type(node, pty::Identifier);
    return dynamic_cast<Identifier*>(node)->get_name();
}

ast::Modifiers visitModifierList(Node* node, ast::Modifiers modifiers) {
    check_node_type(node, pty::ModifierList);
    check_num_children(node, 1, 2);
    if(node->num_children() == 1) {
        modifiers.set(visitModifier(node->child(0)));
        return modifiers;
    } else if(node->num_children() == 2) {
        modifiers = visitModifierList(node->child(0), modifiers);
        modifiers.set(visitModifier(node->child(1)));
        return modifiers;
    }
    unreachable();
}

Modifier visitModifier(Node* node) {
    check_node_type(node, pty::Modifier);
    return dynamic_cast<Modifier*>(node)->get_type();
}

ast::Type* visitType(Node* node) {
    check_num_children(node, 1, 1);
    if(node->get_type() == pty::ArrayType) {

    } else if(node->get_type() == pty::Type) {

    }
    throw std::runtime_error("Expected a Type or ArrayType nod");
}

} // namespace parsetree
