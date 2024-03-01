#include "semantic/TypeResolver.h"
#include "ast/AstNode.h"
#include "ast/ExprNode.h"
#include "ast/Type.h"
#include "diagnostics/Location.h"

namespace semantic {

using namespace ast;
using namespace ast::exprnode;

void ExprTypeResolver::resolve(ast::Expr *expr) {
   loc_ = expr->location();
   this->Evaluate(expr);
}
   
Type* ExprTypeResolver::mapValue(ExprValue& node) const {
   // Implementation goes here
   // ...
   return nullptr;
}

Type* ExprTypeResolver::evalBinaryOp(BinaryOp& op, const Type* lhs, const Type* rhs) const {
   switch(op.getType()) {
      case BinaryOp::OpType::Assignment: {
         // fixme(Owen): TODO
      }

      case BinaryOp::OpType::GreaterThan:
      case BinaryOp::OpType::GreaterThanOrEqual:
      case BinaryOp::OpType::LessThan:
      case BinaryOp::OpType::LessThanOrEqual: {
         if (lhs->isNumeric() && rhs->isNumeric()) {
            return new ast::BuiltInType(ast::BuiltInType::Kind::Boolean, loc_);
         } else {
            throw diag.ReportError(loc_) << "Invalid types for " << BinaryOp::OpType_to_string(op.getType(), "??") << " operation, operands are non-numeric";
         }
      }

      case BinaryOp::OpType::Equal:
      case BinaryOp::OpType::NotEqual: {
         if (lhs->isNumeric() && rhs->isNumeric()) {
            return new ast::BuiltInType(ast::BuiltInType::Kind::Boolean, loc_);
         } else if (lhs->isBoolean() && rhs->isBoolean()) {
            return new ast::BuiltInType(ast::BuiltInType::Kind::Boolean, loc_);
         } 

         auto lhsType = dynamic_cast<const ast::ReferenceType*>(lhs);
         auto rhsType = dynamic_cast<const ast::ReferenceType*>(rhs);
      
         if ((lhs->isNull() || lhsType) && (rhs->isNull() || rhsType)) {
            return new ast::BuiltInType(ast::BuiltInType::Kind::Boolean, loc_);
         } else {
            throw diag.ReportError(loc_) << "Invalid types for " << BinaryOp::OpType_to_string(op.getType(), "??") << " operation, operands are not of the same type";
         }
      }

      case BinaryOp::OpType::Add: {
         if (lhs->isString() || rhs->isString()) {
            return new ast::BuiltInType(ast::BuiltInType::Kind::String, loc_);
         } else if (lhs->isNumeric() && rhs->isNumeric()) {
            return new ast::BuiltInType(ast::BuiltInType::Kind::Int, loc_);
         } else {
            throw diag.ReportError(loc_) << "Invalid types for arithmetic " << BinaryOp::OpType_to_string(op.getType(), "??") << " operation";
         }
      }

      case BinaryOp::OpType::And:
      case BinaryOp::OpType::Or:
      case BinaryOp::OpType::BitwiseAnd:
      case BinaryOp::OpType::BitwiseOr:
      case BinaryOp::OpType::BitwiseXor: {
         if (lhs->isBoolean() && rhs->isBoolean()) {
            return new ast::BuiltInType(ast::BuiltInType::Kind::Boolean, loc_);
         } else {
            throw diag.ReportError(loc_) << "Invalid types for " << BinaryOp::OpType_to_string(op.getType(), "??") << " operation, operands are non-boolean";
         }
      }

      case BinaryOp::OpType::Subtract:
      case BinaryOp::OpType::Multiply:
      case BinaryOp::OpType::Divide:
      case BinaryOp::OpType::Modulo: {
         if (lhs->isNumeric() && rhs->isNumeric()) {
            return new ast::BuiltInType(ast::BuiltInType::Kind::Int, loc_);
         } else {
            throw diag.ReportError(loc_) << "Invalid types for " << BinaryOp::OpType_to_string(op.getType(), "??") << " operation, operands are non-numeric";
         }
      }

      case BinaryOp::OpType::InstanceOf: {
         auto lhsType = dynamic_cast<const ast::ReferenceType*>(lhs);
         auto rhsType = dynamic_cast<const ast::ReferenceType*>(rhs);
      
         if ((lhs->isNull() || lhsType) && (rhs->isNull() || rhsType)) {
            return new ast::BuiltInType(ast::BuiltInType::Kind::Boolean, loc_);
         } else {
            throw diag.ReportError(loc_) << "Invalid types for " << BinaryOp::OpType_to_string(op.getType(), "??") << " operation, operands are null or reference types";
         }
      }

      default:
         return nullptr;
   }
}

Type* ExprTypeResolver::evalUnaryOp(UnaryOp& op, const Type* rhs) const {
   switch(op.getType()) {
      case UnaryOp::OpType::Plus:
      case UnaryOp::OpType::Minus:
      case UnaryOp::OpType::BitwiseNot:
         if (rhs->isNumeric()) {
            return new ast::BuiltInType(ast::BuiltInType::Kind::Int, loc_);
         } else {
            throw diag.ReportError(loc_) << "Invalid type for unary " << UnaryOp::OpType_to_string(op.getType(), "??") << " non-numeric";
         }
      case UnaryOp::OpType::Not:
         if (rhs->isBoolean()) {
            return new ast::BuiltInType(ast::BuiltInType::Kind::Boolean, loc_);
         } else {
            throw diag.ReportError(loc_) << "Invalid type for unary not, non-boolean";
         }
      default:
         return nullptr;
   }
}

Type* ExprTypeResolver::evalMemberAccess(const Type* lhs, const Type* field) const {
   (void) lhs;
   // Fixme(Larry): Look into this later
   return const_cast<Type*>(field);
}

Type* ExprTypeResolver::evalMethodCall(const Type* method, const op_array& args) const {
   // Implementation goes here
   // ...
}

Type* ExprTypeResolver::evalNewObject(const Type* object, const op_array& args) const {
   // Implementation goes here
   // ...
}

Type* ExprTypeResolver::evalNewArray(const Type* type, const Type* size) const {
   if (!size->isNumeric()) {
      throw diag.ReportError(loc_) << "Invalid type for array size, non-numeric";
   }
}

Type* ExprTypeResolver::evalArrayAccess(const Type* array, const Type* index) const {
   if (!index->isNumeric()) {
      throw diag.ReportError(loc_) << "Invalid type for array index, non-numeric";
   }
   if (auto arrayType = dynamic_cast<const ArrayType*>(array)) {
      return arrayType->getElementType();
   } else {
      throw diag.ReportError(loc_) << "Invalid type for array access, non-array";
   }
}

Type* ExprTypeResolver::evalCast(const Type* type, const Type* value) const {
   // Implementation goes here
   // ...
}

} // namespace semantic
