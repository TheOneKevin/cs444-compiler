#include "ast/AST.h"
#include "parsetree/ParseTree.h"
#include "parsetree/ParseTreeVisitor.h"

namespace parsetree {

using pty = Node::Type;
using ptv = ParseTreeVisitor;

// pty::ClassDeclaration ///////////////////////////////////////////////////////

ast::ClassDecl* ptv::visitClassDeclaration(Node* node) {
   check_node_type(node, pty::ClassDeclaration);
   check_num_children(node, 5, 5);
   // $1: Visit the modifiers
   ast::Modifiers modifiers =
         visitModifierList(node->child(0), ast::Modifiers{});
   // $3: Visit identifier
   auto name = visitIdentifier(node->child(1));
   // $4: Visit SuperOpt
   ast::QualifiedIdentifier* super = visitSuperOpt(node->child(2));
   // $5: Visit InterfaceTypeList
   ast::pmr_vector<ast::QualifiedIdentifier*> interfaces;
   visitListPattern<pty::InterfaceTypeList, ast::QualifiedIdentifier*, true>(
         node->child(3), interfaces);
   // $6: Visit ClassBody
   ast::pmr_vector<ast::Decl*> classBodyDeclarations;
   visitListPattern<pty::ClassBodyDeclarationList, ast::Decl*, true>(
         node->child(4), classBodyDeclarations);
   // Return the constructed AST node
   return sem.BuildClassDecl(modifiers,
                             visitIdentifier(node->child(1)),
                             super,
                             interfaces,
                             classBodyDeclarations);
}

ast::QualifiedIdentifier* ptv::visitSuperOpt(Node* node) {
   if(node == nullptr) return nullptr;
   check_node_type(node, pty::SuperOpt);
   check_num_children(node, 1, 1);
   return visitQualifiedIdentifier(node->child(0));
}

template <>
ast::QualifiedIdentifier* ptv::visit<pty::InterfaceTypeList>(Node* node) {
   return visitQualifiedIdentifier(node);
}

template <>
ast::Decl* ptv::visit<pty::ClassBodyDeclarationList>(Node* node) {
   auto nodety = node->get_node_type();
   switch(nodety) {
      case pty::FieldDeclaration:
         return visitFieldDeclaration(node);
      case pty::MethodDeclaration:
         return visitMethodDeclaration(node);
      case pty::ConstructorDeclaration:
         return visitConstructorDeclaration(node);
      default:
         unreachable();
   }
}

// pty::FieldDeclaration ///////////////////////////////////////////////////////

ast::FieldDecl* ptv::visitFieldDeclaration(Node* node) {
   check_node_type(node, pty::FieldDeclaration);
   check_num_children(node, 3, 3);
   // $1: Visit the modifiers
   auto modifiers = visitModifierList(node->child(0));
   // $2: Visit the type
   auto type = visitType(node->child(1));
   // $3: Visit the declarators
   ast::pmr_vector<ast::VarDecl*> declarators;
   visitListPattern<pty::VariableDeclaratorList, ast::VarDecl*, false>(
         node->child(2), declarators);
   return sem.BuildFieldDecl(modifiers, type, "");
}

template <>
ast::VarDecl* ptv::visit<pty::VariableDeclaratorList>(Node* node) {
   check_node_type(node, pty::VariableDeclarator);
   check_num_children(node, 1, 2);
   // FIXME(kevin): Implement this
   return sem.BuildVarDecl(nullptr, "");
}

// pty::MethodDeclaration //////////////////////////////////////////////////////

ast::MethodDecl* ptv::visitMethodDeclaration(Node* node) {
   check_node_type(node, pty::MethodDeclaration);
   check_num_children(node, 2, 2);

   // $1: Visit the header
   auto pt_header = node->child(0);
   check_node_type(pt_header, pty::MethodHeader);
   check_num_children(pt_header, 3, 4);
   ast::Modifiers modifiers;
   ast::Type* type = nullptr;
   std::string name;
   ast::pmr_vector<ast::VarDecl*> params;
   if(pt_header->num_children() == 3) {
      // $1: Visit the modifiers
      modifiers = visitModifierList(pt_header->child(0));
      // The type is void
      type = nullptr;
      // $2: Visit the identifier
      name = visitIdentifier(pt_header->child(1));
      // $3: Visit the formal parameters
      visitListPattern<pty::FormalParameterList, ast::VarDecl*, true>(
            pt_header->child(2), params);
   } else if(pt_header->num_children() == 4) {
      // $1: Visit the modifiers
      modifiers = visitModifierList(pt_header->child(0));
      // $2: Visit the type
      type = visitType(pt_header->child(1));
      // $3: Visit the identifier
      name = visitIdentifier(pt_header->child(2));
      // $4: Visit the formal parameters
      visitListPattern<pty::FormalParameterList, ast::VarDecl*, true>(
            pt_header->child(3), params);
   }

   // $2: Visit the body
   auto pt_body = node->child(1);
   ast::Stmt* body = nullptr;
   if(pt_body != nullptr) {
      body = visitBlock(pt_body);
   }

   // Return the constructed AST node
   return sem.BuildMethodDecl(modifiers, name, type, params, false, body);
}

ast::MethodDecl* ptv::visitConstructorDeclaration(Node* node) {
   check_node_type(node, pty::ConstructorDeclaration);
   check_num_children(node, 4, 4);

   // $1: Visit the modifiers
   auto modifiers = visitModifierList(node->child(0));
   // $2: Visit the identifier
   auto name = visitIdentifier(node->child(1));
   // $3: Visit the formal parameters
   ast::pmr_vector<ast::VarDecl*> params;
   visitListPattern<pty::FormalParameterList, ast::VarDecl*, true>(
         node->child(2), params);
   // $4: Visit the body
   ast::Stmt* body = nullptr;
   if(node->child(3) != nullptr) {
      body = visitBlock(node->child(3));
   }

   return sem.BuildMethodDecl(modifiers, name, nullptr, params, true, body);
}

template <>
ast::VarDecl* ptv::visit<pty::FormalParameterList>(Node* node) {
   check_node_type(node, pty::FormalParameter);
   check_num_children(node, 2, 2);
   // $1: Visit the type
   auto type = visitType(node->child(0));
   // $2: Visit the identifier
   auto name = visitIdentifier(node->child(1));
   // Return the constructed AST node
   return sem.BuildVarDecl(type, name);
}

// pty::InterfaceDeclaration ///////////////////////////////////////////////////

ast::InterfaceDecl* ptv::visitInterfaceDeclaration(Node* node) {
   check_node_type(node, pty::InterfaceDeclaration);
   check_num_children(node, 4, 4);
   // $1: Visit the modifiers
   ast::Modifiers modifiers =
         visitModifierList(node->child(0), ast::Modifiers{});
   // $2: Visit identifier
   auto name = visitIdentifier(node->child(1));
   // $3: Visit ExtendsInterfacesOpt
   ast::pmr_vector<ast::QualifiedIdentifier*> extends;
   visitListPattern<pty::InterfaceTypeList, ast::QualifiedIdentifier*, true>(
         node->child(2), extends);
   // $4: Visit InterfaceBody
   ast::pmr_vector<ast::Decl*> interfaceBodyDeclarations;
   visitListPattern<pty::InterfaceMemberDeclarationList, ast::Decl*, true>(
         node->child(3), interfaceBodyDeclarations);
   // Return the constructed AST node
   return sem.BuildInterfaceDecl(
         modifiers, name, extends, interfaceBodyDeclarations);
}

template <>
ast::Decl* ptv::visit<pty::InterfaceMemberDeclarationList>(Node* node) {
   return visitAbstractMethodDeclaration(node);
}

// pty::AbstractMethodDeclaration //////////////////////////////////////////////

ast::MethodDecl* ptv::visitAbstractMethodDeclaration(Node* node) {
   check_node_type(node, pty::AbstractMethodDeclaration);
   check_num_children(node, 3, 4);
   ast::Modifiers modifiers;
   ast::Type* type = nullptr;
   std::string name;
   ast::pmr_vector<ast::VarDecl*> params;
   if(node->num_children() == 3) {
      // $1: Visit the modifiers
      modifiers = visitModifierList(node->child(0));
      // The type is void
      type = nullptr;
      // $2: Visit the identifier
      name = visitIdentifier(node->child(1));
      // $3: Visit the formal parameters
      visitListPattern<pty::FormalParameterList, ast::VarDecl*, true>(
            node->child(2), params);
   } else if(node->num_children() == 4) {
      // $1: Visit the modifiers
      modifiers = visitModifierList(node->child(0));
      // $2: Visit the type
      type = visitType(node->child(1));
      // $3: Visit the identifier
      name = visitIdentifier(node->child(2));
      // $4: Visit the formal parameters
      visitListPattern<pty::FormalParameterList, ast::VarDecl*, true>(
            node->child(3), params);
   }
   // Set as abstract
   modifiers.set(ast::Modifiers::Type::Abstract);
   // Return the constructed AST node
   return sem.BuildMethodDecl(modifiers, name, type, params, false, nullptr);
}

} // namespace parsetree
