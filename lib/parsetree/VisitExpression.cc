#include "ParseTreeVisitor.h"

namespace parsetree {

using pty = Node::Type;
using ptv = ParseTreeVisitor;
using ast::ExprNodeList;
using namespace ast;
using namespace ast::exprnode;

UnaryOp* ptv::convertToUnaryOp(Operator::Type type) {
   using oty = Operator::Type;
   switch(type) {
      case oty::Not:
         return alloc.new_object<UnaryOp>(UnaryOp::OpType::Not);
      case oty::Minus:
         return alloc.new_object<UnaryOp>(UnaryOp::OpType::Minus);
      default:
         throw std::runtime_error("Invalid operator type");
   }
}

BinaryOp* ptv::convertToBinaryOp(Operator::Type type) {
   using oty = Operator::Type;
   switch(type) {
      case oty::Assign:
         return alloc.new_object<BinaryOp>(BinaryOp::OpType::Assignment);
      case oty::Or:
         return alloc.new_object<BinaryOp>(BinaryOp::OpType::Or);
      case oty::And:
         return alloc.new_object<BinaryOp>(BinaryOp::OpType::And);
      case oty::BitwiseOr:
         return alloc.new_object<BinaryOp>(BinaryOp::OpType::BitwiseOr);
      case oty::BitwiseXor:
         return alloc.new_object<BinaryOp>(BinaryOp::OpType::BitwiseXor);
      case oty::BitwiseAnd:
         return alloc.new_object<BinaryOp>(BinaryOp::OpType::BitwiseAnd);
      case oty::Equal:
         return alloc.new_object<BinaryOp>(BinaryOp::OpType::Equal);
      case oty::NotEqual:
         return alloc.new_object<BinaryOp>(BinaryOp::OpType::NotEqual);
      case oty::LessThan:
         return alloc.new_object<BinaryOp>(BinaryOp::OpType::LessThan);
         break;
      case oty::LessThanOrEqual:
         return alloc.new_object<BinaryOp>(BinaryOp::OpType::LessThanOrEqual);
      case oty::GreaterThan:
         return alloc.new_object<BinaryOp>(BinaryOp::OpType::GreaterThan);
      case oty::GreaterThanOrEqual:
         return alloc.new_object<BinaryOp>(BinaryOp::OpType::GreaterThanOrEqual);
      case oty::InstanceOf:
         return alloc.new_object<BinaryOp>(BinaryOp::OpType::InstanceOf);
      case oty::Plus:
         return alloc.new_object<BinaryOp>(BinaryOp::OpType::Add);
      case oty::Minus:
         return alloc.new_object<BinaryOp>(BinaryOp::OpType::Subtract);
      case oty::Multiply:
         return alloc.new_object<BinaryOp>(BinaryOp::OpType::Multiply);
      case oty::Divide:
         return alloc.new_object<BinaryOp>(BinaryOp::OpType::Divide);
      case oty::Modulo:
         return alloc.new_object<BinaryOp>(BinaryOp::OpType::Modulo);
      default:
         throw std::runtime_error("Invalid operator type");
   }
}

Expr* ptv::visitExpr(Node* node) {
   return new Expr(visitExprChild(node), node->location());
}

ExprNodeList ptv::visitExprNode(parsetree::Node* node) {
   check_node_type(node, pty::Expression);
   check_num_children(node, 1, 3);
   if(node->num_children() == 1) {
      return visitExprChild(node->child(0));
   } else if(node->num_children() == 2) {
      // unary expression
      ExprNodeList ops;
      auto right = visitExprChild(node->child(1));
      ops.concat(right);
      if(auto op = dynamic_cast<parsetree::Operator*>(node->child(0))) {
         ops.push_back(convertToUnaryOp(op->get_type()));
         return ops;
      } else {
         unreachable();
      }
   } else if(node->num_children() == 3) {
      // binary expression
      ExprNodeList ops;
      auto left = visitExprChild(node->child(0));
      auto right = visitExprChild(node->child(2));
      ops.concat(left);
      ops.concat(right);
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
// possible nodes: expression, literal, THIS, qualifiedIdentifier,
// methodInvocation, Type, ArrayType,
//                  arrayAccess, fieldAccess, castExpression,
//                  ArrayCreationExpression ClassInstanceCreationExpression
ExprNodeList ptv::visitExprChild(Node* node) {
   if(node->get_node_type() == pty::Expression) {
      return visitExprNode(node);
   }
   if(node->get_node_type() == pty::Literal) {
      return ExprNodeList(visitLiteral(node));
   }
   if(node->get_node_type() == pty::Type) {
      return ExprNodeList(visitRegularType(node));
   }
   if(node->get_node_type() == pty::ArrayType ||
      node->get_node_type() == pty::ArrayCastType) {
      return ExprNodeList(visitArrayType(node));
   }
   if(node->get_node_type() == pty::Identifier) {
      auto name = visitIdentifier(node);
      if(name == "this") {
         return ExprNodeList(alloc.new_object<ThisNode>());
      }
      return ExprNodeList(alloc.new_object<MemberName>(name));
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

ExprNodeList ptv::visitQualifiedIdentifierInExpr(Node* node,
                                                 bool isMethodInvocation) {
   check_node_type(node, pty::QualifiedIdentifier);
   check_num_children(node, 1, 2);
   ExprNodeList ops;
   if(node->num_children() == 1) {
      if(isMethodInvocation) {
         ops.push_back(
               alloc.new_object<MethodName>(visitIdentifier(node->child(0))));
      } else {
         ops.push_back(
               alloc.new_object<MemberName>(visitIdentifier(node->child(0))));
      }

   } else if(node->num_children() == 2) {
      ops = visitQualifiedIdentifierInExpr(node->child(0));
      if(isMethodInvocation) {
         ops.push_back(
               alloc.new_object<MethodName>(visitIdentifier(node->child(1))));
      } else {
         ops.push_back(
               alloc.new_object<MemberName>(visitIdentifier(node->child(1))));
      }
      ops.push_back(alloc.new_object<MemberAccess>());
   }
   return ops;
}

ExprNodeList ptv::visitMethodInvocation(Node* node) {
   check_node_type(node, pty::MethodInvocation);
   check_num_children(node, 2, 3);
   ExprNodeList ops;
   if(node->num_children() == 2) {
      ops.concat(visitQualifiedIdentifierInExpr(node->child(0), true));

      ExprNodeList args;
      visitArgumentList(node->child(1), args);
      ops.concat(args);

      ops.push_back(alloc.new_object<MethodInvocation>(args.size() + 1));
      return ops;
   } else if(node->num_children() == 3) {
      ops.concat(visitExprChild(node->child(0)));
      ops.push_back(alloc.new_object<MethodName>(visitIdentifier(node->child(1))));
      ops.push_back(alloc.new_object<MemberAccess>());

      ExprNodeList args;
      visitArgumentList(node->child(2), args);
      ops.concat(args);

      ops.push_back(alloc.new_object<MethodInvocation>(args.size() + 1));
      return ops;
   }
   unreachable();
}

ExprNodeList ptv::visitFieldAccess(Node* node) {
   check_node_type(node, pty::FieldAccess);
   check_num_children(node, 2, 2);
   ExprNodeList ops;
   ops.concat(visitExprChild(node->child(0)));
   ops.push_back(alloc.new_object<MemberName>(visitIdentifier(node->child(1))));
   ops.push_back(alloc.new_object<MemberAccess>());
   return ops;
}

ExprNodeList ptv::visitClassCreation(Node* node) {
   check_node_type(node, pty::ClassInstanceCreationExpression);
   check_num_children(node, 2, 2);
   ExprNodeList ops;
   ops.concat(visitQualifiedIdentifierInExpr(node->child(0)));
   ExprNodeList args;
   visitArgumentList(node->child(1), args);
   ops.concat(args);
   ops.push_back(alloc.new_object<ClassInstanceCreation>(args.size() + 1));
   return ops;
}

ExprNodeList ptv::visitArrayAccess(Node* node) {
   check_node_type(node, pty::ArrayAccess);
   check_num_children(node, 2, 2);
   ExprNodeList ops;
   ops.concat(visitExprChild(node->child(0)));
   ops.concat(visitExprChild(node->child(1)));
   ops.push_back(alloc.new_object<ArrayAccess>());
   return ops;
}

ExprNodeList ptv::visitCastExpression(Node* node) {
   check_node_type(node, pty::CastExpression);
   check_num_children(node, 2, 3);
   ExprNodeList ops;
   if(auto basicType = dynamic_cast<parsetree::BasicType*>(node->child(0))) {
      auto type = alloc.new_object<BuiltInType>(basicType->get_type());
      if(node->num_children() == 3 && node->child(1) != nullptr) {
         ops.push_back(alloc.new_object<TypeNode>(
               alloc.new_object<ArrayType>(alloc, type)));
      } else {
         ops.push_back(alloc.new_object<TypeNode>(type));
      }
   } else if(node->child(0)->get_node_type() == pty::QualifiedIdentifier) {
      auto type = visitReferenceType(node->child(0));
      ops.push_back(alloc.new_object<TypeNode>(type));
   } else {
      ops.push_back(visitArrayType(node->child(0)));
   }
   ops.concat(visitExprChild(node->child(node->num_children() - 1)));
   ops.push_back(alloc.new_object<Cast>());
   return ops;
}

ExprNodeList ptv::visitArrayCreation(Node* node) {
   check_node_type(node, pty::ArrayCreationExpression);
   check_num_children(node, 2, 2);
   ExprNodeList ops(visitArrayTypeInExpr(node->child(0)));
   ops.concat(visitExprChild(node->child(1)));
   ops.push_back(alloc.new_object<ArrayInstanceCreation>());
   return ops;
   unreachable();
}

ExprNode* ptv::visitArrayTypeInExpr(Node* node) {
   if(auto basicType = dynamic_cast<parsetree::BasicType*>(node)) {
      auto type = alloc.new_object<BuiltInType>(basicType->get_type());
      return alloc.new_object<TypeNode>(alloc.new_object<ArrayType>(alloc, type));
   } else if(node->get_node_type() == pty::QualifiedIdentifier) {
      auto type = visitReferenceType(node);
      return alloc.new_object<TypeNode>(alloc.new_object<ArrayType>(alloc, type));
   }
   unreachable();
}

ExprNode* ptv::visitRegularType(Node* node) {
   check_node_type(node, pty::Type);
   if(auto basicType = dynamic_cast<parsetree::BasicType*>(node->child(0))) {
      return alloc.new_object<TypeNode>(
            alloc.new_object<BuiltInType>(basicType->get_type()));
   } else if(node->child(0)->get_node_type() == pty::QualifiedIdentifier) {
      return alloc.new_object<TypeNode>(visitReferenceType(node->child(0)));
   }
   unreachable();
}

ExprNode* ptv::visitArrayType(Node* node) {
   if(node->get_node_type() != pty::ArrayType &&
      node->get_node_type() != pty::ArrayCastType) {
      throw ParseTreeException(node,
                               "Called on a node that is not the correct type!"
                               " Expected: " +
                                     node->type_string() +
                                     " Actual: " + node->type_string());
   }
   check_num_children(node, 1, 1);
   return visitArrayTypeInExpr(node->child(0));
}

LiteralNode* ptv::visitLiteral(Node* node) {
   check_node_type(node, pty::Literal);
   if(auto lit = dynamic_cast<parsetree::Literal*>(node)) {
      return alloc.new_object<LiteralNode>(lit->get_value(),
                                           sem.BuildBuiltInType(lit->get_type()));
   }
   unreachable();
}

void ptv::visitArgumentList(Node* node, ExprNodeList& ops) {
   if(node == nullptr) return;
   check_node_type(node, pty::ArgumentList);
   check_num_children(node, 1, 2);
   if(node->num_children() == 1) {
      ops.concat(visitExprChild(node->child(0)));
   } else if(node->num_children() == 2) {
      visitArgumentList(node->child(0), ops);
      ops.concat(visitExprChild(node->child(1)));
   }
   return;
}

} // namespace parsetree
