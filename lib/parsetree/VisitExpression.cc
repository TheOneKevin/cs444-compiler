#include "parsetree/ParseTreeVisitor.h"

namespace parsetree {

using pty = Node::Type;
using ptv = ParseTreeVisitor;
using ast::ExprNodeList;
using namespace ast;
using namespace ast::exprnode;

// NOTE: This is a hack because we want to get the semantic's allocator
#define sem_alloc sem.allocator().new_object

UnaryOp* ptv::convertToUnaryOp(Operator::Type type) {
   using oty = Operator::Type;
   switch(type) {
      case oty::Not:
         return sem_alloc<UnaryOp>(UnaryOp::OpType::Not);
      case oty::Minus:
         return sem_alloc<UnaryOp>(UnaryOp::OpType::Minus);
      default:
         throw std::runtime_error("Invalid operator type");
   }
}

BinaryOp* ptv::convertToBinaryOp(Operator::Type type) {
   using oty = Operator::Type;
   switch(type) {
      case oty::Assign:
         return sem_alloc<BinaryOp>(BinaryOp::OpType::Assignment);
      case oty::Or:
         return sem_alloc<BinaryOp>(BinaryOp::OpType::Or);
      case oty::And:
         return sem_alloc<BinaryOp>(BinaryOp::OpType::And);
      case oty::BitwiseOr:
         return sem_alloc<BinaryOp>(BinaryOp::OpType::BitwiseOr);
      case oty::BitwiseXor:
         return sem_alloc<BinaryOp>(BinaryOp::OpType::BitwiseXor);
      case oty::BitwiseAnd:
         return sem_alloc<BinaryOp>(BinaryOp::OpType::BitwiseAnd);
      case oty::Equal:
         return sem_alloc<BinaryOp>(BinaryOp::OpType::Equal);
      case oty::NotEqual:
         return sem_alloc<BinaryOp>(BinaryOp::OpType::NotEqual);
      case oty::LessThan:
         return sem_alloc<BinaryOp>(BinaryOp::OpType::LessThan);
         break;
      case oty::LessThanOrEqual:
         return sem_alloc<BinaryOp>(BinaryOp::OpType::LessThanOrEqual);
      case oty::GreaterThan:
         return sem_alloc<BinaryOp>(BinaryOp::OpType::GreaterThan);
      case oty::GreaterThanOrEqual:
         return sem_alloc<BinaryOp>(BinaryOp::OpType::GreaterThanOrEqual);
      case oty::InstanceOf:
         return sem_alloc<BinaryOp>(BinaryOp::OpType::InstanceOf);
      case oty::Plus:
         return sem_alloc<BinaryOp>(BinaryOp::OpType::Add);
      case oty::Minus:
         return sem_alloc<BinaryOp>(BinaryOp::OpType::Subtract);
      case oty::Multiply:
         return sem_alloc<BinaryOp>(BinaryOp::OpType::Multiply);
      case oty::Divide:
         return sem_alloc<BinaryOp>(BinaryOp::OpType::Divide);
      case oty::Modulo:
         return sem_alloc<BinaryOp>(BinaryOp::OpType::Modulo);
      default:
         assert(false && "Invalid operator type");
   }
   unreachable();
}

Expr* ptv::visitExpr(Node* node) {
   return sem_alloc<Expr>(visitExprChild(node), node->location());
}

ExprNodeList ptv::visitExprNode(parsetree::Node* node) {
   check_node_type(node, pty::Expression);
   check_num_children(node, 1, 3);
   if(node->num_children() == 1) {
      return visitExprChild(node->child(0));
   } else if(node->num_children() == 2) {
      // unary expression
      ExprNodeList ops{};
      auto right = visitExprChild(node->child(1));
      ops.concat(right);
      auto op = cast<parsetree::Operator>(node->child(0));
      ops.push_back(convertToUnaryOp(op->get_type()));
      return ops;

   } else if(node->num_children() == 3) {
      // binary expression
      ExprNodeList ops{};
      auto left = visitExprChild(node->child(0));
      auto right = visitExprChild(node->child(2));
      ops.concat(left);
      ops.concat(right);
      auto op = cast<parsetree::Operator*>(node->child(1));
      ops.push_back(convertToBinaryOp(op->get_type()));
      return ops;
   }
   unreachable();
}

// expression can have different types of children, so we need to visit them
// possible nodes: expression, literal, THIS, qualifiedIdentifier,
// methodInvocation, Type, ArrayType,
//                  arrayAccess, fieldAccess, castExpression,
//                  ArrayCreationExpression ClassInstanceCreationExpression
ExprNodeList ptv::visitExprChild(Node* node) {
   switch(node->get_node_type()) {
      case pty::Expression:
         return visitExprNode(node);
      case pty::Literal:
         return ExprNodeList(visitLiteral(node));
      case pty::Type:
         return ExprNodeList(visitRegularType(node));
      case pty::ArrayType:
         return ExprNodeList(visitArrayType(node));
      case pty::Identifier: {
         auto name = visitIdentifier(node);
         if(name == "this") {
            return ExprNodeList(sem_alloc<ThisNode>());
         }
         return ExprNodeList(sem_alloc<MemberName>(alloc, name));
      }
      case pty::QualifiedIdentifier:
         return visitQualifiedIdentifierInExpr(node);
      case pty::MethodInvocation:
         return visitMethodInvocation(node);
      case pty::ArrayAccess:
         return visitArrayAccess(node);
      case pty::FieldAccess:
         return visitFieldAccess(node);
      case pty::CastExpression:
         return visitCastExpression(node);
      case pty::ArrayCreationExpression:
         return visitArrayCreation(node);
      case pty::ClassInstanceCreationExpression:
         return visitClassCreation(node);
      default:
         assert(false && "Invalid node type");
   }
   unreachable();
}

ExprNodeList ptv::visitQualifiedIdentifierInExpr(Node* node,
                                                 bool isMethodInvocation) {
   check_node_type(node, pty::QualifiedIdentifier);
   check_num_children(node, 1, 2);
   ExprNodeList ops{};
   if(node->num_children() == 1) {
      if(isMethodInvocation) {
         ops.push_back(
               sem_alloc<MethodName>(alloc, visitIdentifier(node->child(0))));
      } else {
         ops.push_back(
               sem_alloc<MemberName>(alloc, visitIdentifier(node->child(0))));
      }

   } else if(node->num_children() == 2) {
      ops = visitQualifiedIdentifierInExpr(node->child(0));
      if(isMethodInvocation) {
         ops.push_back(
               sem_alloc<MethodName>(alloc, visitIdentifier(node->child(1))));
      } else {
         ops.push_back(
               sem_alloc<MemberName>(alloc, visitIdentifier(node->child(1))));
      }
      ops.push_back(sem_alloc<MemberAccess>());
   }
   return ops;
}

ExprNodeList ptv::visitMethodInvocation(Node* node) {
   check_node_type(node, pty::MethodInvocation);
   check_num_children(node, 2, 3);
   ExprNodeList ops{};
   if(node->num_children() == 2) {
      ops.concat(visitQualifiedIdentifierInExpr(node->child(0), true));

      ExprNodeList args{};
      auto size = visitArgumentList(node->child(1), args) + 1;
      ops.concat(args);

      ops.push_back(sem_alloc<MethodInvocation>(size));
      return ops;
   } else if(node->num_children() == 3) {
      ops.concat(visitExprChild(node->child(0)));
      ops.push_back(sem_alloc<MethodName>(alloc, visitIdentifier(node->child(1))));
      ops.push_back(sem_alloc<MemberAccess>());

      ExprNodeList args{};
      auto size = visitArgumentList(node->child(2), args) + 1;
      ops.concat(args);

      ops.push_back(sem_alloc<MethodInvocation>(size));
      return ops;
   }
   unreachable();
}

ExprNodeList ptv::visitFieldAccess(Node* node) {
   check_node_type(node, pty::FieldAccess);
   check_num_children(node, 2, 2);
   ExprNodeList ops{};
   ops.concat(visitExprChild(node->child(0)));
   ops.push_back(sem_alloc<MemberName>(alloc, visitIdentifier(node->child(1))));
   ops.push_back(sem_alloc<MemberAccess>());
   return ops;
}

ExprNodeList ptv::visitClassCreation(Node* node) {
   check_node_type(node, pty::ClassInstanceCreationExpression);
   check_num_children(node, 2, 2);
   ExprNodeList ops{};
   auto type = visitReferenceType(node->child(0));
   ops.push_back(sem_alloc<TypeNode>(type));
   ExprNodeList args{};
   auto size = visitArgumentList(node->child(1), args) + 1;
   ops.concat(args);
   ops.push_back(sem_alloc<ClassInstanceCreation>(size));
   return ops;
}

ExprNodeList ptv::visitArrayAccess(Node* node) {
   check_node_type(node, pty::ArrayAccess);
   check_num_children(node, 2, 2);
   ExprNodeList ops{};
   ops.concat(visitExprChild(node->child(0)));
   ops.concat(visitExprChild(node->child(1)));
   ops.push_back(sem_alloc<ArrayAccess>());
   return ops;
}

ExprNodeList ptv::visitCastExpression(Node* node) {
   check_node_type(node, pty::CastExpression);
   check_num_children(node, 2, 3);
   ExprNodeList ops{};
   Node* expr;
   // If there are 3 children, the expr is (type, dims, expr)
   // Otherwise, the expr is (type, expr)
   if(node->num_children() == 3) {
      auto type = visitType(node->child(0));
      auto dims = node->child(1);
      if(dims) {
         type = sem.BuildArrayType(type, type->location());
      }
      ops.push_back(sem_alloc<TypeNode>(type));
      expr = node->child(2);
   } else {
      auto type = visitType(node->child(0));
      ops.push_back(sem_alloc<TypeNode>(type));
      expr = node->child(1);
   }
   ops.concat(visitExprChild(expr));
   ops.push_back(sem_alloc<Cast>());
   return ops;
}

ExprNodeList ptv::visitArrayCreation(Node* node) {
   check_node_type(node, pty::ArrayCreationExpression);
   check_num_children(node, 2, 2);
   ExprNodeList ops(visitArrayType(node->child(0)));
   ops.concat(visitExprChild(node->child(1)));
   ops.push_back(sem_alloc<ArrayInstanceCreation>());
   return ops;
   unreachable();
}

ExprNode* ptv::visitRegularType(Node* node) {
   return sem_alloc<TypeNode>(visitType(node));
}

ExprNode* ptv::visitArrayType(Node* node) {
   check_node_type(node, pty::ArrayType);
   return sem_alloc<TypeNode>(visitType(node));
}

LiteralNode* ptv::visitLiteral(Node* node) {
   check_node_type(node, pty::Literal);
   auto lit = cast<parsetree::Literal>(node);
   return sem_alloc<LiteralNode>(
         sem.allocator(), lit->get_value(), sem.BuildBuiltInType(lit->get_type()));
}

int ptv::visitArgumentList(Node* node, ExprNodeList& ops) {
   if(node == nullptr) return 0;
   check_node_type(node, pty::ArgumentList);
   check_num_children(node, 1, 2);
   int args = -1;
   if(node->num_children() == 1) {
      args = 1;
      ops.concat(visitExprChild(node->child(0)));
   } else if(node->num_children() == 2) {
      args = visitArgumentList(node->child(0), ops) + 1;
      ops.concat(visitExprChild(node->child(1)));
   }
   return args;
}

} // namespace parsetree
