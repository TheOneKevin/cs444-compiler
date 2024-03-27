#include "semantic/ConstantTypeResolver.h"

#include "ast/AstNode.h"
#include "ast/ExprNode.h"
#include "ast/Type.h"
#include "utils/Utils.h"

namespace semantic {

using namespace ast;
using namespace ast::exprnode;

ConstantReturnType const* ConstantTypeResolver::mapValue(ExprValue& node) const {
   if(auto literal = dyn_cast<LiteralNode>(&node)) {
      ast::BuiltInType const* type = literal->builtinType();
      if (type->isNumeric()) {
         return alloc.new_object<ConstantReturnType>(ConstantReturnType::type::INT, literal->getAsInt());
      } else if (type->isBoolean()) {
         return alloc.new_object<ConstantReturnType>(ConstantReturnType::type::BOOL, literal->getAsInt());
      }
   }

   return alloc.new_object<ConstantReturnType>(ConstantReturnType::type::UNKNOWN, 0);
}

ConstantReturnType const* ConstantTypeResolver::evalBinaryOp(BinaryOp& op, const ConstantReturnType* lhs,
                                           const ConstantReturnType* rhs) const {
   switch(op.opType()) {
      case BinaryOp::OpType::Assignment: {
         return rhs;
      }

      case BinaryOp::OpType::GreaterThan: 
      case BinaryOp::OpType::GreaterThanOrEqual:
      case BinaryOp::OpType::LessThan:
      case BinaryOp::OpType::LessThanOrEqual: {
         if (lhs->constantType == ConstantReturnType::type::INT && rhs->constantType == ConstantReturnType::type::INT) {
            switch(op.opType()) {
               case BinaryOp::OpType::GreaterThan: 
                  return alloc.new_object<ConstantReturnType>(ConstantReturnType::type::BOOL, lhs->value > rhs->value);
               case BinaryOp::OpType::GreaterThanOrEqual:
                  return alloc.new_object<ConstantReturnType>(ConstantReturnType::type::BOOL, lhs->value >= rhs->value);
               case BinaryOp::OpType::LessThan:
                  return alloc.new_object<ConstantReturnType>(ConstantReturnType::type::BOOL, lhs->value < rhs->value);
               case BinaryOp::OpType::LessThanOrEqual:
                  return alloc.new_object<ConstantReturnType>(ConstantReturnType::type::BOOL, lhs->value <= rhs->value);
               default: assert(false && "invalid binary operation");
            }
         }
         
         return alloc.new_object<ConstantReturnType>(ConstantReturnType::type::UNKNOWN, 0);
      }

      case BinaryOp::OpType::Equal:
      case BinaryOp::OpType::NotEqual: {
         if (lhs->constantType == ConstantReturnType::type::UNKNOWN ||
             rhs->constantType == ConstantReturnType::type::UNKNOWN) {
            return alloc.new_object<ConstantReturnType>(ConstantReturnType::type::UNKNOWN, 0);
         }

         switch(op.opType()) {
            case BinaryOp::OpType::Equal: 
               return alloc.new_object<ConstantReturnType>(ConstantReturnType::type::BOOL, lhs->value == rhs->value);
            case BinaryOp::OpType::NotEqual:
               return alloc.new_object<ConstantReturnType>(ConstantReturnType::type::BOOL, lhs->value != rhs->value);
            default: assert(false && "invalid binary operation");
         }
      }

      case BinaryOp::OpType::And:
      case BinaryOp::OpType::Or:
      case BinaryOp::OpType::BitwiseAnd:
      case BinaryOp::OpType::BitwiseOr:{
         if (lhs->constantType == ConstantReturnType::type::BOOL && rhs->constantType == ConstantReturnType::type::BOOL) {
            assert(lhs->value == 0 || lhs->value == 1 && "invalid value for boolean");
            assert(rhs->value == 0 || rhs->value == 1 && "invalid value for boolean");
            
            switch(op.opType()) {
               case BinaryOp::OpType::And:
               case BinaryOp::OpType::BitwiseAnd:
                  return alloc.new_object<ConstantReturnType>(ConstantReturnType::type::BOOL, lhs->value && rhs->value);
               case BinaryOp::OpType::Or:
               case BinaryOp::OpType::BitwiseOr:
                  return alloc.new_object<ConstantReturnType>(ConstantReturnType::type::BOOL, lhs->value || rhs->value);
               default: assert(false && "invalid binary operation");
            }
         }
         
         return alloc.new_object<ConstantReturnType>(ConstantReturnType::type::UNKNOWN, 0);
      }
      
      case BinaryOp::OpType::BitwiseXor: 
         assert(false && "We don't support bitwise xor operations");

      case BinaryOp::OpType::Add:
      case BinaryOp::OpType::Subtract:
      case BinaryOp::OpType::Multiply:
      case BinaryOp::OpType::Divide:
      case BinaryOp::OpType::Modulo: {
         if (lhs->constantType == ConstantReturnType::type::INT && rhs->constantType == ConstantReturnType::type::INT) {
            switch(op.opType()) {
               case BinaryOp::OpType::Add:
                  return alloc.new_object<ConstantReturnType>(ConstantReturnType::type::INT, lhs->value + rhs->value);
               case BinaryOp::OpType::Subtract:
                  return alloc.new_object<ConstantReturnType>(ConstantReturnType::type::INT, lhs->value - rhs->value);
               case BinaryOp::OpType::Multiply:
                  return alloc.new_object<ConstantReturnType>(ConstantReturnType::type::INT, lhs->value * rhs->value);
               case BinaryOp::OpType::Divide:
                  return alloc.new_object<ConstantReturnType>(ConstantReturnType::type::INT, lhs->value / rhs->value);
               case BinaryOp::OpType::Modulo:
                  return alloc.new_object<ConstantReturnType>(ConstantReturnType::type::INT, lhs->value % rhs->value);
               default: assert(false && "invalid binary operation");
            }   
         }
              
         return alloc.new_object<ConstantReturnType>(ConstantReturnType::type::UNKNOWN, 0);
      }

      case BinaryOp::OpType::InstanceOf: {
         return alloc.new_object<ConstantReturnType>(ConstantReturnType::type::UNKNOWN, 0);
      }

      default:
         assert(false && "invalid binary operation");
   }
}

ConstantReturnType const* ConstantTypeResolver::evalUnaryOp(UnaryOp& op, const ConstantReturnType* rhs) const {
   switch(op.opType()) {
      case UnaryOp::OpType::Plus: {
         return rhs;
      }
      case UnaryOp::OpType::Minus: {
         return alloc.new_object<ConstantReturnType>(ConstantReturnType::type::INT, -rhs->value);
      }
      case UnaryOp::OpType::BitwiseNot: 
      case UnaryOp::OpType::Not: {
         if (rhs->constantType == ConstantReturnType::type::BOOL) {
            assert(rhs->value == 0 || rhs->value == 1 && "invalid value for boolean");
            return alloc.new_object<ConstantReturnType>(ConstantReturnType::type::BOOL, !rhs->value);
         }

         return alloc.new_object<ConstantReturnType>(ConstantReturnType::type::UNKNOWN, 0);
      }
      default: assert(false && "invalid unary operation");
   }
}

ConstantReturnType const* ConstantTypeResolver::evalMemberAccess(DotOp& op, const ConstantReturnType*,
                                               const ConstantReturnType* field) const {
   (void)op;
   (void)field;
   return alloc.new_object<ConstantReturnType>(ConstantReturnType::type::UNKNOWN, -1);
}

ConstantReturnType const* ConstantTypeResolver::evalMethodCall(MethodOp& op, const ConstantReturnType* method,
                                             const op_array& args) const {
   (void)op;
   (void)method;
   (void)args;
   return alloc.new_object<ConstantReturnType>(ConstantReturnType::type::UNKNOWN, -1);
}

ConstantReturnType const* ConstantTypeResolver::evalNewObject(NewOp& op, const ConstantReturnType* object,
                                            const op_array& args) const {
   (void)op;
   (void)object;
   (void)args;
   return alloc.new_object<ConstantReturnType>(ConstantReturnType::type::UNKNOWN, -1);
}

ConstantReturnType const* ConstantTypeResolver::evalNewArray(NewArrayOp& op, const ConstantReturnType* array,
                                           const ConstantReturnType* size) const {
   (void)op;
   (void)array;
   (void)size;
   return alloc.new_object<ConstantReturnType>(ConstantReturnType::type::UNKNOWN, -1);
}

ConstantReturnType const* ConstantTypeResolver::evalArrayAccess(ArrayAccessOp& op, const ConstantReturnType* array,
                                              const ConstantReturnType* index) const {
   (void)op;
   (void)array;
   (void)index;
   return alloc.new_object<ConstantReturnType>(ConstantReturnType::type::UNKNOWN, -1);
}

ConstantReturnType const* ConstantTypeResolver::evalCast(CastOp& op, const ConstantReturnType* type,
                                       const ConstantReturnType* value) const {
   (void)op;
   (void)type;
   return value;
}

} // namespace semantic
