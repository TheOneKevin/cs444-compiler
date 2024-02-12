#include "ParseTreeVisitor.h"
#include <list>

namespace parsetree {

using pty = Node::Type;
using opType = Operator::Type;

std::list<ast::ExprNode> visitStatementExpression(Node* node) {
   check_node_type(node, pty::StatementExpression);
   check_num_children(node, 1, 1);
}

ast::BinaryOp convertToBinaryOp(opType type) {
   switch(type) {
      case opType::Assign:
         return ast::BinaryOp(ast::BinaryOp::OpType::Assignment);
      case opType::Or:
         return ast::BinaryOp(ast::BinaryOp::OpType::Or);
      case opType::And:
         return ast::BinaryOp(ast::BinaryOp::OpType::And);
      case opType::BitwiseOr:
         return ast::BinaryOp(ast::BinaryOp::OpType::BitwiseOr);
      case opType::BitwiseXor:
         return ast::BinaryOp(ast::BinaryOp::OpType::BitwiseXor);
      case opType::BitwiseAnd:
         return ast::BinaryOp(ast::BinaryOp::OpType::BitwiseAnd);
      case opType::Equal:
         return ast::BinaryOp(ast::BinaryOp::OpType::Equal);
      case opType::NotEqual:
         return ast::BinaryOp(ast::BinaryOp::OpType::NotEqual);
      case opType::LessThan:
         return ast::BinaryOp(ast::BinaryOp::OpType::LessThan);
         break;
      case opType::LessThanOrEqual:
         return ast::BinaryOp(ast::BinaryOp::OpType::LessThanOrEqual);
      case opType::GreaterThan:
         return ast::BinaryOp(ast::BinaryOp::OpType::GreaterThan);
      case opType::GreaterThanOrEqual:
         return ast::BinaryOp(ast::BinaryOp::OpType::GreaterThanOrEqual);
      case opType::InstanceOf:
         return ast::BinaryOp(ast::BinaryOp::OpType::InstanceOf);
      case opType::Add:
         return ast::BinaryOp(ast::BinaryOp::OpType::Add);
      case opType::Subtract:
         return ast::BinaryOp(ast::BinaryOp::OpType::Subtract);
      case opType::Multiply:
         return ast::BinaryOp(ast::BinaryOp::OpType::Multiply);
      case opType::Divide:
         return ast::BinaryOp(ast::BinaryOp::OpType::Divide);
      case opType::Modulo:
         return ast::BinaryOp(ast::BinaryOp::OpType::Modulo);
   }
}

std::list<ast::ExprNode> visitExpr(parsetree::Node* node) {
   check_node_type(node, pty::Expression);
   check_num_children(node, 1, 3);
   if(node->num_children() == 1) {
      return visitExpr(node->child(0));
   } else if(node->num_children() == 2) {
      // unary expression, add later
   } else if(node->num_children() == 3) {
      // binary expression
      std::list<ast::ExprNode> ops;
      auto left = visitExprChild(node->child(0));
      auto right = visitExprChild(node->child(2));
      ops.splice(ops.end(), left);
      ops.splice(ops.end(), right);
      if(auto op = dynamic_cast<parsetree::Operator*>(node->child(1))) {
         ops.push_back(convertToBinaryOp(op->get_type()));
         return ops;
      } else {
         unreachable();
      }
   }
   unreachable();
}

// expression can have different types of children, so we need to visit them
// possible nodes: expression, literal, THIS, qualifiedIdentifier, methodInvocation,
//                  arrayAccess, fieldAccess, castExpression, ArrayCreationExpression
//                  ClassInstanceCreationExpression
std::list<ast::ExprNode> visitExprChild(Node* node) {
   if(node->get_node_type() == pty::Expression) {
      return visitExpr(node);
   }
   if(node->get_node_type() == pty::Literal) {
      
   }
   if(node->get_node_type() == pty::QualifiedIdentifier) {
      return visitQualifiedIdentifierInExpr(node);
   }
   if(node->get_node_type() == pty::MethodInvocation) {
      return visitMethodInvocation(node);
   }
   if(node->get_node_type() == pty::ArrayAccess) {
      return visitArrayAccess(node);
   }
   if(node->get_node_type() == pty::FieldAccess) {
      return visitFieldAccess(node);
   }
   if(node->get_node_type() == pty::CastExpression) {
      return visitCastExpression(node);
   }
   if(node->get_node_type() == pty::ArrayCreationExpression) {
      return visitArrayCreation(node);
   }
   if(node->get_node_type() == pty::ClassInstanceCreationExpression) {
      return visitClassCreation(node);
   }
   unreachable();
}

std::list<ast::ExprNode> visitQualifiedIdentifierInExpr(Node* node) {
   check_node_type(node, pty::QualifiedIdentifier);
   check_num_children(node, 1, 2);
   std::list<ast::ExprNode> ops;
   if(node->num_children() == 1) {
      ops.push_back(ast::MemberName(visitIdentifier(node->child(1))));
   } else if(node->num_children() == 2) {
      ops = visitQualifiedIdentifierInExpr(node->child(0));
      ops.push_back(ast::MemberName(visitIdentifier(node->child(1))));
      ops.push_back(ast::MemberAccess());
   }
   return ops;
}

std::list<ast::ExprNode> visitMethodInvocation(Node* node) {
   check_node_type(node, pty::MethodInvocation);
   check_num_children(node, 2, 3);
   std::list<ast::ExprNode> ops;
   if(node->num_children() == 2) {
      ops.splice(ops.end(), visitQualifiedIdentifierInExpr(node->child(0)));

      std::list<ast::ExprNode> args;
      visitArgumentList(node->child(1), args);
      ops.splice(ops.end(), args);

      ops.push_back(ast::MethodInvocation(args.size() + 1));
      return ops;
   } else if(node->num_children() == 3) {
      ops.splice(ops.end(), visitExprChild(node->child(0)));
      ops.push_back(ast::MemberName(visitIdentifier(node->child(1))));
      ops.push_back(ast::MemberAccess());

      std::list<ast::ExprNode> args;
      visitArgumentList(node->child(2), args);
      ops.splice(ops.end(), args);

      ops.push_back(ast::MethodInvocation(args.size() + 1));
      return ops;
   }
   unreachable();
}

std::list<ast::ExprNode> visitFieldAccess(Node* node) {
   check_node_type(node, pty::FieldAccess);
   check_num_children(node, 2, 2);
   std::list<ast::ExprNode> ops;
   ops.splice(ops.end(), visitExprChild(node->child(0)));
   ops.push_back(ast::MemberName(visitIdentifier(node->child(1))));
   ops.push_back(ast::MemberAccess());
   return ops;
}

std::list<ast::ExprNode> visitClassCreation(Node* node) {
   check_node_type(node, pty::ClassInstanceCreationExpression);
   check_num_children(node, 2, 2);
   std::list<ast::ExprNode> ops;
   ops.push_back(ast::MemberName(visitIdentifier(node->child(0))));
   std::list<ast::ExprNode> args;
   visitArgumentList(node->child(1), args);
   ops.splice(ops.end(), args);
   ops.push_back(ast::ClassInstanceCreation(args.size() + 1));
   return ops;
}

std::list<ast::ExprNode> visitArrayAccess(Node* node) {
   check_node_type(node, pty::ArrayAccess);
   check_num_children(node, 2, 2);
   std::list<ast::ExprNode> ops;
   ops.splice(ops.end(), visitExprChild(node->child(0)));
   ops.splice(ops.end(), visitExpr(node->child(1)));
   ops.push_back(ast::ArrayAccess());
   return ops;
}

std::list<ast::ExprNode> visitArrayAccess(Node* node) {
   check_node_type(node, pty::ArrayAccess);
   check_num_children(node, 2, 2);
   std::list<ast::ExprNode> ops;
   ops.splice(ops.end(), visitExprChild(node->child(0)));
   ops.splice(ops.end(), visitExpr(node->child(1)));
   ops.push_back(ast::ArrayAccess());
   return ops;
}

void visitArgumentList(Node* node, std::list<ast::ExprNode>& ops) {
   if (node == nullptr) return;
   check_node_type(node, pty::ArgumentList);
   check_num_children(node, 1, 2);
   if(node->num_children() == 1) {
      ops.splice(ops.end(), visitExpr(node->child(0)));
   } else if(node->num_children() == 2) {
      visitArgumentList(node->child(0), ops);
      ops.splice(ops.end(), visitExpr(node->child(1)));
   }
   return;
}



} // namespace parsetree
