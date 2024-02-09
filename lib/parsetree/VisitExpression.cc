#include "ParseTreeVisitor.h"

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
// possible nodes: expression, literal, qualifiedIdentifier, methodInvocation, 
//                  arrayAccess, fieldAccess, castExpression, this, ClassInstanceCreationExpression
std::list<ast::ExprNode> visitExprChild(Node* node) {
    if (node->get_node_type() == pty::Expression) {
        return visitExpr(node);
    } 
    if (node->get_node_type() == pty::Literal) {

    }
    unreachable();
}

} // namespace parsetree
