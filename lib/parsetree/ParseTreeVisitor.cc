#include "ParseTreeVisitor.h"

#include "ParseTree.h"
#include "ast/AST.h"

namespace parsetree {

using pty = Node::Type;
using ptv = ParseTreeVisitor;

ast::CompilationUnit* ptv::visitCompilationUnit(Node* node) {
   // Check the node we're visiting
   check_node_type(node, pty::CompilationUnit);
   check_num_children(node, 3, 3);
   // $1: Visit the package declaration
   auto package = visitPackageDeclaration(node->child(0));
   // $2: Visit the import declarations
   ast::pmr_vector<ast::ImportDeclaration> imports;
   visitListPattern<pty::ImportDeclarationList, ast::ImportDeclaration, true>(
         node->child(1), imports);
   // $3: Visit the body, if it is not null
   if(auto body = node->child(2)) {
      if(body->get_node_type() == pty::ClassDeclaration) {
         ast::ClassDecl* class_body = visitClassDeclaration(body);
         return sem.BuildCompilationUnit(
               package, imports, class_body->location(), class_body);
      } else if(body->get_node_type() == pty::InterfaceDeclaration) {
         ast::InterfaceDecl* intf_body = visitInterfaceDeclaration(body);
         return sem.BuildCompilationUnit(
               package, imports, intf_body->location(), intf_body);
      }
   }
   return nullptr;
}

ast::ReferenceType* ptv::visitPackageDeclaration(Node* node) {
   if(node == nullptr) return sem.BuildUnresolvedType();
   check_node_type(node, pty::PackageDeclaration);
   check_num_children(node, 1, 1);
   return visitReferenceType(node->child(0));
}

template <>
ast::ImportDeclaration ptv::visit<pty::ImportDeclarationList>(Node* node) {
   check_num_children(node, 1, 1);
   auto id = visitReferenceType(node->child(0));
   if(node->get_node_type() == pty::SingleTypeImportDeclaration) {
      return ast::ImportDeclaration{id, false};
   } else if(node->get_node_type() == pty::TypeImportOnDemandDeclaration) {
      return ast::ImportDeclaration{id, true};
   }
   throw std::runtime_error("Import called on a node that is not a Import");
}

} // namespace parsetree
