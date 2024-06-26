#include "parsetree/ParseTree.h"
#include "parsetree/ParseTreeVisitor.h"
#include <utils/Error.h>
#include <utility>

namespace parsetree {

using pty = Node::Type;
using ptv = ParseTreeVisitor;

ast::UnresolvedType* ptv::visitReferenceType(Node* node,
                                             ast::UnresolvedType* ast_node) {
   check_node_type(node, pty::QualifiedIdentifier);
   check_num_children(node, 1, 2);
   if(ast_node == nullptr) {
      ast_node = sem.BuildUnresolvedType(node->location());
   }
   if(node->num_children() == 1) {
      ast_node->addIdentifier(visitIdentifier(node->child(0)));
      return ast_node;
   } else if(node->num_children() == 2) {
      ast_node = visitReferenceType(node->child(0), ast_node);
      ast_node->addIdentifier(visitIdentifier(node->child(1)));
      return ast_node;
   }
   std::unreachable();
}

std::string_view ptv::visitIdentifier(Node* node) {
   check_node_type(node, pty::Identifier);
   return cast<Identifier*>(node)->get_name();
}

ast::Modifiers ptv::visitModifierList(Node* node, ast::Modifiers modifiers) {
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
   std::unreachable();
}

Modifier ptv::visitModifier(Node* node) {
   check_node_type(node, pty::Modifier);
   return *cast<Modifier*>(node);
}

ast::Type* ptv::visitType(Node* node) {
   check_num_children(node, 1, 1);
   auto innerTy = node;
   innerTy = node->child(0);
   ast::Type* elemTy = nullptr;
   if(innerTy->get_node_type() == pty::BasicType) {
      elemTy = sem.BuildBuiltInType(cast<BasicType*>(innerTy)->get_type(),
                                    node->location());
   } else if(innerTy->get_node_type() == pty::QualifiedIdentifier) {
      elemTy = visitReferenceType(innerTy);
   }
   if(elemTy == nullptr)
      throw utils::FatalError(
            "Expected a BasicType or QualifiedIdentifier node but got " +
            innerTy->type_string());
   if(node->get_node_type() == pty::ArrayType)
      return sem.BuildArrayType(elemTy, node->location());
   else if(node->get_node_type() == pty::Type)
      return elemTy;
   throw utils::FatalError("Expected a Type or ArrayType node");
}

} // namespace parsetree
