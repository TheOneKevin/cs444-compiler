#include <list>

#include "ParseTreeVisitor.h"

namespace parsetree {

using pty = Node::Type;
using ptv = ParseTreeVisitor;

ast::UnaryOp* ptv::convertToUnaryOp(Operator::Type type) {
   using oty = Operator::Type;
   switch(type) {
      case oty::Not:
         return alloc.new_object<ast::UnaryOp>(ast::UnaryOp::OpType::Not);
      case oty::Minus:
         return alloc.new_object<ast::UnaryOp>(ast::UnaryOp::OpType::Minus);
   } 
   throw std::runtime_error("Invalid operator type");
}

ast::BinaryOp* ptv::convertToBinaryOp(Operator::Type type) {
   using oty = Operator::Type;
   switch(type) {
      case oty::Assign:
         return alloc.new_object<ast::BinaryOp>(ast::BinaryOp::OpType::Assignment);
      case oty::Or:
         return alloc.new_object<ast::BinaryOp>(ast::BinaryOp::OpType::Or);
      case oty::And:
         return alloc.new_object<ast::BinaryOp>(ast::BinaryOp::OpType::And);
      case oty::BitwiseOr:
         return alloc.new_object<ast::BinaryOp>(ast::BinaryOp::OpType::BitwiseOr);
      case oty::BitwiseXor:
         return alloc.new_object<ast::BinaryOp>(ast::BinaryOp::OpType::BitwiseXor);
      case oty::BitwiseAnd:
         return alloc.new_object<ast::BinaryOp>(ast::BinaryOp::OpType::BitwiseAnd);
      case oty::Equal:
         return alloc.new_object<ast::BinaryOp>(ast::BinaryOp::OpType::Equal);
      case oty::NotEqual:
         return alloc.new_object<ast::BinaryOp>(ast::BinaryOp::OpType::NotEqual);
      case oty::LessThan:
         return alloc.new_object<ast::BinaryOp>(ast::BinaryOp::OpType::LessThan);
         break;
      case oty::LessThanOrEqual:
         return alloc.new_object<ast::BinaryOp>(ast::BinaryOp::OpType::LessThanOrEqual);
      case oty::GreaterThan:
         return alloc.new_object<ast::BinaryOp>(ast::BinaryOp::OpType::GreaterThan);
      case oty::GreaterThanOrEqual:
         return alloc.new_object<ast::BinaryOp>(ast::BinaryOp::OpType::GreaterThanOrEqual);
      case oty::InstanceOf:
         return alloc.new_object<ast::BinaryOp>(ast::BinaryOp::OpType::InstanceOf);
      case oty::Add:
         return alloc.new_object<ast::BinaryOp>(ast::BinaryOp::OpType::Add);
      case oty::Subtract:
         return alloc.new_object<ast::BinaryOp>(ast::BinaryOp::OpType::Subtract);
      case oty::Multiply:
         return alloc.new_object<ast::BinaryOp>(ast::BinaryOp::OpType::Multiply);
      case oty::Divide:
         return alloc.new_object<ast::BinaryOp>(ast::BinaryOp::OpType::Divide);
      case oty::Modulo:
         return alloc.new_object<ast::BinaryOp>(ast::BinaryOp::OpType::Modulo);
   }
   throw std::runtime_error("Invalid operator type");
}

ast::Expr* ptv::visitExpr(Node* node) {
   return new ast::Expr(visitExprChild(node));
}

std::list<ast::ExprNode*> ptv::visitExprNode(parsetree::Node* node) {
   check_node_type(node, pty::Expression);
   check_num_children(node, 1, 3);
   if(node->num_children() == 1) {
      return visitExprChild(node->child(0));
   } else if(node->num_children() == 2) {
      // unary expression
      std::list<ast::ExprNode*> ops;
      auto right = visitExprChild(node->child(1));
      ops.splice(ops.end(), right);
      if(auto op = dynamic_cast<parsetree::Operator*>(node->child(0))) {
         ops.push_back(convertToUnaryOp(op->get_type()));
         return ops;
      } else {
         unreachable();
      }
   } else if(node->num_children() == 3) {
      // binary expression
      std::list<ast::ExprNode*> ops;
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
// possible nodes: expression, literal, THIS, qualifiedIdentifier,
// methodInvocation, Type, ArrayType,
//                  arrayAccess, fieldAccess, castExpression,
//                  ArrayCreationExpression ClassInstanceCreationExpression
std::list<ast::ExprNode*> ptv::visitExprChild(Node* node) {
   if(node->get_node_type() == pty::Expression) {
      return visitExprNode(node);
   }
   if(node->get_node_type() == pty::Literal) {
      return std::list<ast::ExprNode*>({visitLiteral(node)});
   }
   if (node->get_node_type() == pty::Type) {
      return visitTypeInExpr(node);
   }
   if (node->get_node_type() == pty::ArrayType) {
      return visitArrayType(node);
   }
   if(node->get_node_type() == pty::Identifier) {
      auto name = visitIdentifier(node);
      if (name == "this") {
         return std::list<ast::ExprNode*>({alloc.new_object<ast::ThisNode>()});
      }
      return std::list<ast::ExprNode*>({alloc.new_object<ast::MemberName>(name)});
   }
   if(node->get_node_type() == pty::QualifiedIdentifier) {
      return visitQualifiedIdentifierInExpr(node);
   }
   if (node->get_node_type() == pty::ArrayCastType) {
      return visitQualifiedIdentifierInExpr(node->child(0));
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

std::list<ast::ExprNode*> ptv::visitQualifiedIdentifierInExpr(Node* node) {
   check_node_type(node, pty::QualifiedIdentifier);
   check_num_children(node, 1, 2);
   std::list<ast::ExprNode*> ops;
   if(node->num_children() == 1) {
      ops.push_back(alloc.new_object<ast::MemberName>(visitIdentifier(node->child(0))));
   } else if(node->num_children() == 2) {
      ops = visitQualifiedIdentifierInExpr(node->child(0));
      ops.push_back(alloc.new_object<ast::MemberName>(visitIdentifier(node->child(1))));
      ops.push_back(alloc.new_object<ast::MemberAccess>());
   }
   return ops;
}

std::list<ast::ExprNode*> ptv::visitMethodInvocation(Node* node) {
   check_node_type(node, pty::MethodInvocation);
   check_num_children(node, 2, 3);
   std::list<ast::ExprNode*> ops;
   if(node->num_children() == 2) {
      ops.splice(ops.end(), visitQualifiedIdentifierInExpr(node->child(0)));

      std::list<ast::ExprNode*> args;
      visitArgumentList(node->child(1), args);
      ops.splice(ops.end(), args);

      ops.push_back(alloc.new_object<ast::MethodInvocation>(args.size() + 1));
      return ops;
   } else if(node->num_children() == 3) {
      ops.splice(ops.end(), visitExprChild(node->child(0)));
      ops.push_back(alloc.new_object<ast::MemberName>(visitIdentifier(node->child(1))));
      ops.push_back(alloc.new_object<ast::MemberAccess>());

      std::list<ast::ExprNode*> args;
      visitArgumentList(node->child(2), args);
      ops.splice(ops.end(), args);

      ops.push_back(alloc.new_object<ast::MethodInvocation>(args.size() + 1));
      return ops;
   }
   unreachable();
}

std::list<ast::ExprNode*> ptv::visitFieldAccess(Node* node) {
   check_node_type(node, pty::FieldAccess);
   check_num_children(node, 2, 2);
   std::list<ast::ExprNode*> ops;
   ops.splice(ops.end(), visitExprChild(node->child(0)));
   ops.push_back(alloc.new_object<ast::MemberName>(visitIdentifier(node->child(1))));
   ops.push_back(alloc.new_object<ast::MemberAccess>());
   return ops;
}

std::list<ast::ExprNode*> ptv::visitClassCreation(Node* node) {
   check_node_type(node, pty::ClassInstanceCreationExpression);
   check_num_children(node, 2, 2);
   std::list<ast::ExprNode*> ops;
   ops.push_back(alloc.new_object<ast::MemberName>(visitIdentifier(node->child(0))));
   std::list<ast::ExprNode*> args;
   visitArgumentList(node->child(1), args);
   ops.splice(ops.end(), args);
   ops.push_back(alloc.new_object<ast::ClassInstanceCreation>(args.size() + 1));
   return ops;
}

std::list<ast::ExprNode*> ptv::visitArrayAccess(Node* node) {
   check_node_type(node, pty::ArrayAccess);
   check_num_children(node, 2, 2);
   std::list<ast::ExprNode*> ops;
   ops.splice(ops.end(), visitExprChild(node->child(0)));
   ops.splice(ops.end(), visitExprChild(node->child(1)));
   ops.push_back(alloc.new_object<ast::ArrayAccess>());
   return ops;
}

std::list<ast::ExprNode*> ptv::visitCastExpression(Node* node) {
   check_node_type(node, pty::CastExpression);
   check_num_children(node, 2, 3);
   std::list<ast::ExprNode*> ops;
   auto type = visitExprChild(node->child(0));
   ops.splice(ops.end(), type);
   if (node->num_children() == 3 && node->child(1) != nullptr) {
      ops.push_back(alloc.new_object<ast::ArrayTypeNode>());
   }
   auto expr = visitExprChild(node->child(node->num_children() - 1));
   ops.splice(ops.end(), type);
   ops.push_back(alloc.new_object<ast::Cast>());
   return ops;
}

std::list<ast::ExprNode*> ptv::visitArrayCreation(Node* node) {
   check_node_type(node, pty::ArrayCreationExpression);
   check_num_children(node, 2, 2);
   if (node->child(0)->get_node_type() == pty::QualifiedIdentifier) {
      auto ops = visitQualifiedIdentifierInExpr(node->child(0));
      ops.splice(ops.end(), visitExprChild(node->child(1)));
      ops.push_back(alloc.new_object<ast::ArrayInstanceCreation>());
      return ops;
   } else if (auto basicType = dynamic_cast<parsetree::BasicType*>(node->child(0))) {
      ast::BuiltInType* type = alloc.new_object<ast::BuiltInType>(basicType->get_type());
      std::list<ast::ExprNode*> ops({alloc.new_object<ast::BasicTypeNode>(type)});
      ops.splice(ops.end(), visitExprChild(node->child(1)));
      ops.push_back(alloc.new_object<ast::ArrayInstanceCreation>());
      return ops;
   }
   unreachable();

}

std::list<ast::ExprNode*> ptv::visitTypeInExpr(Node* node) {
   check_node_type(node, pty::Type);
   check_num_children(node, 1, 1);
   if (node->child(0)->get_node_type() == pty::QualifiedIdentifier) {
      return visitQualifiedIdentifierInExpr(node->child(0));
   } else if (auto basicType = dynamic_cast<parsetree::BasicType*>(node->child(0))) {
      ast::BuiltInType type = ast::BuiltInType(basicType->get_type());
      return std::list<ast::ExprNode*>({alloc.new_object<ast::BasicTypeNode>(&type)});
   }
   unreachable();
}

std::list<ast::ExprNode*> ptv::visitArrayType(Node* node) {
   check_node_type(node, pty::ArrayType);
   check_num_children(node, 1, 1);
   if (node->child(0)->get_node_type() == pty::QualifiedIdentifier) {
      auto ops = visitQualifiedIdentifierInExpr(node->child(0));
      ops.push_back(alloc.new_object<ast::ArrayTypeNode>());
      return ops;
   } else if (auto basicType = dynamic_cast<parsetree::BasicType*>(node->child(0))) {
      ast::BuiltInType* type = alloc.new_object<ast::BuiltInType>(basicType->get_type());
      std::list<ast::ExprNode*> ops({alloc.new_object<ast::BasicTypeNode>(type)});
      ops.push_back(alloc.new_object<ast::ArrayTypeNode>());
      return ops;
   }
   unreachable();
}

ast::LiteralNode* ptv::visitLiteral(Node* node) {
   check_node_type(node, pty::Literal);
   if (auto lit = dynamic_cast<parsetree::Literal*>(node)) {
      switch (lit->get_type()) {
         case parsetree::Literal::Type::Integer:
            return alloc.new_object<ast::LiteralNode>(lit->get_value(), ast::LiteralNode::Type::Integer);
         case parsetree::Literal::Type::Character:
            return alloc.new_object<ast::LiteralNode>(lit->get_value(), ast::LiteralNode::Type::Character);
         case parsetree::Literal::Type::String:
            return alloc.new_object<ast::LiteralNode>(lit->get_value(), ast::LiteralNode::Type::String);
         case parsetree::Literal::Type::Boolean:
            return alloc.new_object<ast::LiteralNode>(lit->get_value(), ast::LiteralNode::Type::Boolean);
         case parsetree::Literal::Type::Null:
            return alloc.new_object<ast::LiteralNode>(lit->get_value(), ast::LiteralNode::Type::Null);
      }
   } 
   unreachable();
}

void ptv::visitArgumentList(Node* node, std::list<ast::ExprNode*>& ops) {
   if(node == nullptr) return;
   check_node_type(node, pty::ArgumentList);
   check_num_children(node, 1, 2);
   if(node->num_children() == 1) {
      ops.splice(ops.end(), visitExprChild(node->child(0)));
   } else if(node->num_children() == 2) {
      visitArgumentList(node->child(0), ops);
      ops.splice(ops.end(), visitExprChild(node->child(1)));
   }
   return;
}

} // namespace parsetree
