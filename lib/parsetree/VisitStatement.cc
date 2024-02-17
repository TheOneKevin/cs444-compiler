#include "ast/AST.h"
#include "parsetree/ParseTree.h"
#include "parsetree/ParseTreeVisitor.h"

namespace parsetree {

using pty = Node::Type;
using ptv = ParseTreeVisitor;

// pty::Block //////////////////////////////////////////////////////////////////

ast::BlockStatement* ptv::visitBlock(Node* node) {
   check_node_type(node, pty::Block);
   check_num_children(node, 1, 1);
   ast::pmr_vector<ast::Stmt*> stmts;
   auto scope = sem.EnterLexicalScope();
   visitListPattern<pty::BlockStatementList, ast::Stmt*, true>(node->child(0),
                                                               stmts);
   sem.ExitLexicalScope(scope);
   return sem.BuildBlockStatement(stmts);
}

template <>
ast::Stmt* ptv::visit<pty::BlockStatementList>(Node* node) {
   return visitStatement(node);
}

// pty::Statement //////////////////////////////////////////////////////////////

ast::Stmt* ptv::visitStatement(Node* node) {
   check_node_type(node, pty::Statement);
   if(node->child(0) == nullptr) return sem.BuildNullStmt();
   check_num_children(node, 1, 1);
   auto child = node->child(0);
   auto nodety = child->get_node_type();
   switch(nodety) {
      case pty::Block:
         return visitBlock(child);
      case pty::IfThenStatement:
         return visitIfThenStatement(child);
      case pty::WhileStatement:
         return visitWhileStatement(child);
      case pty::ForStatement:
         return visitForStatement(child);
      case pty::ReturnStatement:
         return visitReturnStatement(child);
      case pty::LocalVariableDeclaration:
         return visitLocalVariableDeclarationStatement(child);
      case pty::StatementExpression:
         return visitExpressionStatement(child);
      default:
         unreachable();
   }
   return nullptr;
}

// pty::IfThenStatement ////////////////////////////////////////////////////////

ast::IfStmt* ptv::visitIfThenStatement(Node* node) {
   check_node_type(node, pty::IfThenStatement);
   check_num_children(node, 2, 3);

   if(node->child(0) == nullptr || node->child(1) == nullptr)
      throw std::runtime_error("Invalid if-then statement");

   // $1: Visit the expression
   auto expr = visitExpr(node->child(0));
   // $2: Visit the statement
   auto scope = sem.EnterLexicalScope();
   auto stmt = visitStatement(node->child(1));
   sem.ExitLexicalScope(scope);

   if(node->num_children() == 3) {
      auto scope = sem.EnterLexicalScope();
      auto elseStmt = visitStatement(node->child(2));
      sem.ExitLexicalScope(scope);
      return sem.BuildIfStmt(expr, stmt, elseStmt);
   }

   return sem.BuildIfStmt(expr, stmt);
}

// pty::WhileStatement /////////////////////////////////////////////////////////

ast::WhileStmt* ptv::visitWhileStatement(Node* node) {
   check_node_type(node, pty::WhileStatement);
   check_num_children(node, 2, 2);

   if(node->child(0) == nullptr || node->child(1) == nullptr)
      throw std::runtime_error("Invalid while statement");

   // $1: Visit the condition
   auto condition = visitExpr(node->child(0));
   // $2: Visit the body
   auto scope = sem.EnterLexicalScope();
   auto body = visitStatement(node->child(1));
   sem.ExitLexicalScope(scope);
   return sem.BuildWhileStmt(condition, body);
}

// pty::ForStatement ///////////////////////////////////////////////////////////

ast::ForStmt* ptv::visitForStatement(Node* node) {
   check_node_type(node, pty::ForStatement);
   check_num_children(node, 4, 4);

   if(node->child(3) == nullptr) throw std::runtime_error("Invalid for statement");

   ast::Stmt* init = nullptr;
   ast::Expr* condition = nullptr;
   ast::Stmt* update = nullptr;
   ast::Stmt* body = nullptr;

   auto scope = sem.EnterLexicalScope();
   if(node->child(0) != nullptr) init = visitStatement(node->child(0));
   if(node->child(1) != nullptr) condition = visitExpr(node->child(1));
   if(node->child(2) != nullptr) update = visitStatement(node->child(2));
   body = visitStatement(node->child(3));
   sem.ExitLexicalScope(scope);

   return sem.BuildForStmt(init, condition, update, body);
}

// pty::ReturnStatement ////////////////////////////////////////////////////////

ast::ReturnStmt* ptv::visitReturnStatement(Node* node) {
   check_node_type(node, pty::ReturnStatement);
   check_num_children(node, 0, 1);
   if(node->child(0) == nullptr) {
      return sem.BuildReturnStmt(nullptr);
   }
   auto expr = visitExpr(node->child(0));
   return sem.BuildReturnStmt(expr);
}

// pty::ExpressionStatement ////////////////////////////////////////////////////

ast::ExprStmt* ptv::visitExpressionStatement(Node* node) {
   check_node_type(node, pty::StatementExpression);
   check_num_children(node, 1, 1);
   auto expr = visitExpr(node->child(0));
   return sem.BuildExprStmt(expr);
}

// pty::VariableDeclarator /////////////////////////////////////////////////////

ptv::TmpVarDecl ptv::visitVariableDeclarator(Node* tyNode, Node* declNode) {
   check_node_type(declNode, pty::VariableDeclarator);
   check_num_children(declNode, 1, 2);
   // $?: Get the type of the variable
   auto type = visitType(tyNode);
   // $1: Get the name of the variable
   auto nameNode = declNode->child(0);
   auto name = visitIdentifier(nameNode);
   // $2: Get the initializer of the variable
   ast::Expr* init = nullptr;
   if(declNode->num_children() == 2) {
      init = visitExpr(declNode->child(1));
   }
   return TmpVarDecl{type, nameNode->location(), name, init};
}

// pty::LocalVariableDeclarationStatement //////////////////////////////////////

ast::DeclStmt* ptv::visitLocalVariableDeclarationStatement(Node* node) {
   check_node_type(node, pty::LocalVariableDeclaration);
   check_num_children(node, 2, 2);
   // $0: Get the type of the variable
   auto tyNode = node->child(0);
   // $1: Get the variable declarator
   auto declNode = node->child(1);
   auto decl = visitVariableDeclarator(tyNode, declNode);
   auto astDecl = sem.BuildVarDecl(decl.type, decl.loc, decl.name, decl.init);
   return sem.BuildDeclStmt(astDecl);
}

} // namespace parsetree
