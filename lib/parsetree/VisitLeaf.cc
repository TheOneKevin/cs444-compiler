#include "ParseTree.h"
#include "ParseTreeVisitor.h"

namespace parsetree {

using pty = Node::Type;

ast::QualifiedIdentifier* visitQualifiedIdentifier(
      Node* node, ast::QualifiedIdentifier* ast_node) {
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

std::string visitIdentifier(Node* node) {
   check_node_type(node, pty::Identifier);
   return dynamic_cast<Identifier*>(node)->get_name();
}

ast::Modifiers visitModifierList(Node* node, ast::Modifiers modifiers) {
   if(node == nullptr) {
      return modifiers;
   }
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
   return *dynamic_cast<Modifier*>(node);
}

ast::Type* visitType(Node* node) {
   check_num_children(node, 1, 1);
   auto innerTy = node;
   innerTy = node->child(0);
   ast::Type* elemTy = nullptr;
   if(innerTy->get_node_type() == pty::BasicType) {
      elemTy =
            new ast::BuiltInType{dynamic_cast<BasicType*>(innerTy)->get_type()};
   } else if(innerTy->get_node_type() == pty::QualifiedIdentifier) {
      elemTy = new ast::ReferenceType{visitQualifiedIdentifier(innerTy)};
   }
   if(elemTy == nullptr)
      throw std::runtime_error(
            "Expected a BasicType or QualifiedIdentifier node but got " +
            innerTy->type_string());
   if(node->get_node_type() == pty::ArrayType)
      return new ast::ArrayType{elemTy};
   else if(node->get_node_type() == pty::Type)
      return elemTy;
   throw std::runtime_error("Expected a Type or ArrayType node");
}

} // namespace parsetree
