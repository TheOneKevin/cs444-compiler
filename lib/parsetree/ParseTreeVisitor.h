#pragma once

#include <list>

#include "ast/AST.h"
#include "parsetree/ParseTree.h"
#include "semantic/Semantic.h"

namespace parsetree {

class ParseTreeException : public std::exception {
public:
   ParseTreeException(Node* where, const std::string& what)
         : msg{what}, where{where} {}

   const char* what() const noexcept override { return msg.c_str(); }

   Node* get_where() const { return where; }

private:
   std::string msg;
   Node* where;
};

class ParseTreeVisitor {
   using pty = Node::Type;

public:
   ParseTreeVisitor(ast::Semantic& sem) : sem{sem}, alloc{sem.getAllocator()} {}

private:
   // Basic helper functions ///////////////////////////////////////////////////

   static inline void check_node_type(Node* node, Node::Type type) {
      if(node->get_node_type() != type) {
         throw ParseTreeException(
               node,
               "Called on a node that is not the correct type!"
               " Expected: " +
                     Node::type_string(type) +
                     " Actual: " + node->type_string());
      }
   }

   static inline void check_num_children(Node* node, size_t min, size_t max) {
      if(node->num_children() < min || node->num_children() > max) {
         throw std::runtime_error(
               "Node has incorrect number of children!"
               " Type: " +
               node->type_string() + " Expected: " + std::to_string(min) +
               " to " + std::to_string(max) +
               " Actual: " + std::to_string(node->num_children()));
      }
   }

   [[noreturn]] static inline void unreachable() {
      throw std::runtime_error("Unreachable code reached!");
   }

   // Templated visitor patterns ///////////////////////////////////////////////

   // NOTE(kevin): Technically we can re-implement all our visitors using the
   // template patterns. However, the syntax is ugly at best. We only use these
   // for the list patterns, which are more tedious and benefit from templates.

   /**
    * @brief This function is not intended to be called.
    * Instead, the function should to be template-specialized to be used by the
    * visitListPattern<N, T> function.
    *
    * @tparam N This parameter must match the N parameter of the
    * visitListPattern function that calls this function.
    * @tparam T This parameter must match the T parameter of the
    * visitListPattern function that calls this function.
    * @param node The node to visit.
    * @return T The result of visiting the node.
    */
   template <parsetree::Node::Type N, typename T>
   T visit(Node* node) {
      throw std::runtime_error("No visitor for node type " +
                               node->type_string());
   }

   /**
    * @brief Visits a list-pattern node. A list pattern is a node that is
    * recursive in the first-child. The node either has 1 or 2 children.
    * If the node has 1 child, the list is simply the result of visiting the
    * child. The visitor will be implemented by the visit<N, T> function
    * (which you need to specialize in order to use this function).
    *
    * @tparam N The type of the node to visit.
    * @tparam T The type of the list elements.
    * @tparam nullable If true, the node can be null (for empty lists).
    * @param node The node to visit.
    * @param list The list to append the results to.
    */
   template <parsetree::Node::Type N, typename T, bool nullable = false>
   void visitListPattern(Node* node, ast::array_ref<T> list) {
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

public:
   // Compilation unit visitors ////////////////////////////////////////////////

   ast::CompilationUnit* visitCompilationUnit(Node* node);
   ast::ReferenceType* visitPackageDeclaration(Node* node);
   template <>
   ast::ImportDeclaration visit<pty::ImportDeclarationList>(Node* node);

   // Classes & interfaces visitors ////////////////////////////////////////////

   ast::ClassDecl* visitClassDeclaration(Node* node);
   ast::InterfaceDecl* visitInterfaceDeclaration(Node* node);
   ast::ReferenceType* visitSuperOpt(Node* node);
   ast::FieldDecl* visitFieldDeclaration(Node* node);
   ast::MethodDecl* visitMethodDeclaration(Node* node);
   ast::MethodDecl* visitConstructorDeclaration(Node* node);
   ast::MethodDecl* visitAbstractMethodDeclaration(Node* node);

   template <>
   ast::Decl* visit<pty::ClassBodyDeclarationList>(Node* node);
   template <>
   ast::VarDecl* visit<pty::FormalParameterList>(Node* node);
   template <>
   ast::Decl* visit<pty::InterfaceMemberDeclarationList>(Node* node);

   // Statements visitors //////////////////////////////////////////////////////

   struct TmpVarDecl {
      ast::Type* type;
      std::string_view name;
      ast::Expr* init;
   };
   static_assert(std::is_standard_layout_v<TmpVarDecl>);
   static_assert(std::is_trivially_copyable_v<TmpVarDecl>);

   TmpVarDecl visitVariableDeclarator(Node* ty, Node* node);
   ast::DeclStmt* visitLocalVariableDeclarationStatement(Node* node);

   ast::BlockStatement* visitBlock(Node* node);
   ast::Stmt* visitStatement(Node* node);
   ast::IfStmt* visitIfThenStatement(Node* node);
   ast::WhileStmt* visitWhileStatement(Node* node);
   ast::ForStmt* visitForStatement(Node* node);
   ast::ReturnStmt* visitReturnStatement(Node* node);
   ast::ExprStmt* visitExpressionStatement(Node* node);

   // Expression visitors //////////////////////////////////////////////////////

   ast::Expr* visitExpr(Node* node);
   std::list<ast::ExprNode*> visitExprChild(Node* node);
   std::list<ast::ExprNode*> visitExprNode(Node* node);
   std::list<ast::ExprNode*> visitMethodInvocation(Node* node);
   std::list<ast::ExprNode*> visitQualifiedIdentifierInExpr(Node* node);
   std::list<ast::ExprNode*> visitArgumentList(Node* node);
   std::list<ast::ExprNode*> visitFieldAccess(Node* node);
   std::list<ast::ExprNode*> visitClassCreation(Node* node);
   std::list<ast::ExprNode*> visitArrayAccess(Node* node);
   std::list<ast::ExprNode*> visitArrayCreation(Node* node);
   std::list<ast::ExprNode*> visitCastExpression(Node* node);
   std::list<ast::ExprNode*> visitTypeInExpr(Node* node);
   std::list<ast::ExprNode*> visitArrayType(Node* node);

   ast::LiteralNode *visitLiteral(Node* node);

   ast::UnaryOp* convertToUnaryOp(Operator::Type type);
   ast::BinaryOp* convertToBinaryOp(Operator::Type type);
   void visitArgumentList(Node* node, std::list<ast::ExprNode*>& ops);

   template <>
   ast::Stmt* visit<pty::BlockStatementList>(Node* node);

   // Leaf node visitors ///////////////////////////////////////////////////////

   ast::UnresolvedType* visitReferenceType(
         Node* node, ast::UnresolvedType* ast_node = nullptr);
   std::string_view visitIdentifier(Node* node);
   ast::Modifiers visitModifierList(
         Node* node, ast::Modifiers modifiers = ast::Modifiers{});
   Modifier visitModifier(Node* node);
   ast::Type* visitType(Node* node);

private:
   ast::Semantic& sem;
   BumpAllocator& alloc;
};

} // namespace parsetree
