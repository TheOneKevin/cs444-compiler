#include "semantic/ExprTypeResolver.h"

#include <algorithm>
#include <iterator>
#include <ranges>
#include <utility>

#include "ast/AstNode.h"
#include "ast/DeclContext.h"
#include "ast/Expr.h"
#include "ast/ExprNode.h"
#include "ast/Type.h"
#include "diagnostics/Location.h"
#include "semantic/NameResolver.h"
#include "utils/Utils.h"

namespace semantic {

using namespace ast;
using namespace ast::exprnode;

// 5.1.2 Widening Primitive Conversion
static bool isWiderThan(const BuiltInType* type, const BuiltInType* other) {
   using Kind = ast::BuiltInType::Kind;
   if(other->getKind() == Kind::Char || other->getKind() == Kind::Short) {
      return type->getKind() == Kind::Int;
   }
   if(other->getKind() == Kind::Byte) {
      return type->getKind() == Kind::Short || type->getKind() == Kind::Int;
   }
   return false;
}

bool ExprTypeResolver::isTypeString(const Type* type) const {
   if(type->isString()) return true;
   if(auto refType = dyn_cast<const ReferenceType*>(type)) {
      return refType->decl() == NR->GetJavaLang().String;
   }
   return false;
}

bool ExprTypeResolver::isReferenceOrArrType(const Type* type) const {
   return dyn_cast<ReferenceType>(type) || dyn_cast<ArrayType>(type) ||
          type->isString();
}

// 1. Identity conversion
// 2. Widening Primitive Conversion
//    2.1 Null type can be cast to any class type, interface type, or array type.
// 3. Widening Reference Conversions
//    3.1 Class type to any super class, or interface that it implements.
//    3.2 Interface type to any super interface OR Object class
//    3.3 Array type
//       3.3.1 Array type to Object class
//       3.3.2 Array type to Cloneable interface
//       3.3.3 Array type to java.io.Serializable interface
//       3.3.4 Array type to another array type given the element type is a
//       widening REFERENCE conversion
bool ExprTypeResolver::isAssignableTo(const Type* lhs, const Type* rhs) const {
   // step 1
   if(*lhs == *rhs) return true;

   auto leftPrimitive = dyn_cast<const ast::BuiltInType*>(lhs);
   auto rightPrimitive = dyn_cast<const ast::BuiltInType*>(rhs);
   auto leftRef = dyn_cast<const ast::ReferenceType*>(lhs);
   auto rightRef = dyn_cast<const ast::ReferenceType*>(rhs);
   auto leftArr = dyn_cast<const ast::ArrayType*>(lhs);
   auto rightArr = dyn_cast<const ast::ArrayType*>(rhs);

   // identity conversion: java.lang.String <-> primitive type string
   if(isTypeString(lhs) && isTypeString(rhs)) return true;

   if(rhs->isString() && leftRef) {
      if(auto leftClass = dyn_cast<ClassDecl const*>(leftRef->decl())) {
         return HC->isSuperClass(leftClass, NR->GetJavaLang().String);
      } else if(auto leftInterface =
                      dyn_cast<InterfaceDecl const*>(leftRef->decl())) {
         return HC->isSuperInterface(leftInterface, NR->GetJavaLang().String);
      } else
         return false;
   }
   if(lhs->isString() && rightRef) {
      if(auto rightClass = dyn_cast<ClassDecl const*>(rightRef->decl())) {
         return HC->isSuperClass(NR->GetJavaLang().String, rightClass);
      } else
         return false;
   }
   // step 2
   if(lhs->isPrimitive() && rhs->isPrimitive()) {
      return isWiderThan(leftPrimitive, rightPrimitive);
   }
   // Step 2.1
   if(rhs->isNull()) {
      return isReferenceOrArrType(lhs);
   }

   if(leftRef && rightRef) {
      if(auto rightClass = dyn_cast<ClassDecl const*>(rightRef->decl())) {
         // Step 3.1
         if(auto leftClass = dyn_cast<ClassDecl const*>(leftRef->decl())) {
            return HC->isSuperClass(leftClass, rightClass);
         } else if(auto leftInterface =
                         dyn_cast<InterfaceDecl const*>(leftRef->decl())) {
            return HC->isSuperInterface(leftInterface, rightClass);
         } else {
            std::unreachable();
         }
      } else if(auto rightInterface =
                      dyn_cast<InterfaceDecl const*>(rightRef->decl())) {
         // Step 3.2
         if(auto leftClass = dyn_cast<ClassDecl const*>(leftRef->decl())) {
            return leftClass = NR->GetJavaLang().Object;
         } else if(auto leftInterface =
                         dyn_cast<InterfaceDecl const*>(leftRef->decl())) {
            return HC->isSuperInterface(leftInterface, rightInterface);
         } else {
            std::unreachable();
         }
      }
   }

   if(rightArr) {
      if(leftArr) {
         // Step 3.3.4
         auto leftElem = dyn_cast<ReferenceType*>(leftArr->getElementType());
         auto rightElem = dyn_cast<ReferenceType*>(rightArr->getElementType());
         return leftElem && rightElem && isAssignableTo(leftElem, rightElem);
      } else if(leftRef) {
         // Step 3.3.1
         if(leftRef->decl() == NR->GetJavaLang().Object) {
            return true;
         }
         // Step 3.3.2
         if(leftRef->decl() == NR->GetJavaLang().Cloneable) {
            return true;
         }
         // Step 3.3.3
         if(leftRef->decl() == NR->GetJavaLang().Serializable) {
            return true;
         }
      }
   }
   return false;
}

bool ExprTypeResolver::isValidCast(const Type* exprType,
                                   const Type* castType) const {
   if(*exprType == *castType) return true;

   // identity conversion: java.lang.String <-> primitive type string
   if(isTypeString(castType) && (isTypeString(exprType) || exprType->isNull())) {
      return true;
   }

   if(isAssignableTo(exprType, castType) || isAssignableTo(castType, exprType)) {
      return true;
   }

   auto exprRef = dyn_cast<ast::ReferenceType*>(exprType);
   auto castRef = dyn_cast<ast::ReferenceType*>(castType);

   // If expr is "null", the type is actually Object
   if(exprType->isNull()) {
      return castRef;
   } else if(castType->isNull()) {
      return exprRef;
   }

   auto exprArr = dyn_cast<ast::ArrayType*>(exprType);
   auto castArr = dyn_cast<ast::ArrayType*>(castType);

   if(exprType->isPrimitive() && castType->isPrimitive()) {
      return exprType->isNumeric() && castType->isNumeric();
   }
   if(exprRef) {
      if(castArr) {
         return exprRef->decl() == NR->GetJavaLang().Object;
      }
      if(!castRef) {
         return false;
      }
      auto leftInterface = dyn_cast<ast::InterfaceDecl*>(exprRef->decl());
      auto rightInterface = dyn_cast<ast::InterfaceDecl*>(castRef->decl());

      auto leftClass = dyn_cast<ast::ClassDecl*>(exprRef->decl());
      auto rightClass = dyn_cast<ast::ClassDecl*>(castRef->decl());

      if(leftInterface && rightInterface) {
         return true;
      } else if(leftInterface && !rightClass->modifiers().isFinal()) {
         return true;
      } else if(rightInterface && !leftClass->modifiers().isFinal()) {
         return true;
      } else {
         return isAssignableTo(exprRef, castRef) ||
                isAssignableTo(castRef, exprRef);
      }
   } else if(exprArr) {
      if(castArr) {
         auto leftElem = dyn_cast<ReferenceType>(exprArr->getElementType());
         auto rightElem = dyn_cast<ReferenceType>(castArr->getElementType());
         return leftElem && rightElem &&
                isValidCast(exprArr->getElementType(), castArr->getElementType());
      } else {
         if(castRef->decl() == NR->GetJavaLang().Object) {
            return true;
         }
         if(castRef->decl() == NR->GetJavaLang().Serializable) {
            return true;
         }
      }
   }
   throw diag.ReportError(loc_)
         << "invalid cast from " << exprType->toString() << " to "
         << castType->toString() << argLocation(0) << "is type "
         << exprType->toString() << argLocation(1) << "is type "
         << castType->toString();
}

Type const* ExprTypeResolver::mapValue(ExprValue& node) const {
   assert(node.isDeclResolved() && "ExprValue decl is not resolved");
   if(auto method = dyn_cast_or_null<MethodDecl>(node.decl())) {
      auto ty = alloc.new_object<MethodType>(alloc, method);
      if(method->isConstructor())
         ty->setReturnType(sema.BuildReferenceType(cast<Decl>(method->parent())));
      return ty;
   } else {
      assert(node.isTypeResolved() && "ExprValue type is not resolved");
      return node.type();
   }
}

Type const* ExprTypeResolver::evalBinaryOp(BinaryOp& op, const Type* lhs,
                                           const Type* rhs) const {
   if(op.resultType()) return op.resultType();
   switch(op.opType()) {
      case BinaryOp::OpType::Assignment: {
         if(isAssignableTo(lhs, rhs)) {
            return op.resolveResultType(lhs);
         }
         throw diag.ReportError(loc_)
               << "invalid assignment: " << lhs->toString()
               << " is not assignable to " << rhs->toString() << argLocation(1)
               << "is type " << rhs->toString() << argLocation(0) << "is type "
               << lhs->toString();
      }

      case BinaryOp::OpType::GreaterThan:
      case BinaryOp::OpType::GreaterThanOrEqual:
      case BinaryOp::OpType::LessThan:
      case BinaryOp::OpType::LessThanOrEqual: {
         if(lhs->isNumeric() && rhs->isNumeric()) {
            return op.resolveResultType(
                  sema.BuildBuiltInType(ast::BuiltInType::Kind::Boolean));
         }
         throw diag.ReportError(loc_)
               << "invalid types for "
               << BinaryOp::OpType_to_string(op.opType(), "??")
               << " operation, operands are non-numeric" << argLocation(1)
               << "is type " << rhs->toString() << argLocation(0) << "is type "
               << lhs->toString();
      }

      case BinaryOp::OpType::Equal:
      case BinaryOp::OpType::NotEqual: {
         if(lhs->isNumeric() && rhs->isNumeric()) {
            return op.resolveResultType(
                  sema.BuildBuiltInType(ast::BuiltInType::Kind::Boolean));
         } else if(lhs->isBoolean() && rhs->isBoolean()) {
            return op.resolveResultType(
                  sema.BuildBuiltInType(ast::BuiltInType::Kind::Boolean));
         }

         auto lhsType = dynamic_cast<const ast::ReferenceType*>(lhs);
         auto rhsType = dynamic_cast<const ast::ReferenceType*>(rhs);

         if((isTypeString(lhs) && rhs->isNull()) || (isTypeString(rhs) && lhs->isNull()) ||
            ((lhs->isNull() || lhsType) && (rhs->isNull() || rhsType) &&
            (isValidCast(lhs, rhs) || isValidCast(rhs, lhs)))) {
            return op.resolveResultType(
                  sema.BuildBuiltInType(ast::BuiltInType::Kind::Boolean));
         }
         throw diag.ReportError(loc_)
               << "invalid types for "
               << BinaryOp::OpType_to_string(op.opType(), "??")
               << " operation, operands are not of the same type" << argLocation(1)
               << "is type " << rhs->toString() << argLocation(0) << "is type "
               << lhs->toString();
      }

      case BinaryOp::OpType::Add: {
         // FIXME(larry) for primitive types, we need to create a new class
         // instance.
         if(isTypeString(lhs) || isTypeString(rhs)) {
            return op.resolveResultType(
                  sema.BuildBuiltInType(ast::BuiltInType::Kind::String));
         } else if(lhs->isNumeric() && rhs->isNumeric()) {
            return op.resolveResultType(
                  sema.BuildBuiltInType(ast::BuiltInType::Kind::Int));
         }
         throw diag.ReportError(loc_)
               << "invalid types for arithmetic "
               << BinaryOp::OpType_to_string(op.opType(), "??") << " operation"
               << argLocation(1) << "is type " << rhs->toString() << argLocation(0)
               << "is type " << lhs->toString();
      }

      case BinaryOp::OpType::And:
      case BinaryOp::OpType::Or:
      case BinaryOp::OpType::BitwiseAnd:
      case BinaryOp::OpType::BitwiseOr: {
         if(lhs->isBoolean() && rhs->isBoolean()) {
            return op.resolveResultType(
                  sema.BuildBuiltInType(ast::BuiltInType::Kind::Boolean));
         }
         throw diag.ReportError(loc_)
               << "invalid types for "
               << BinaryOp::OpType_to_string(op.opType(), "??")
               << " operation, operands are non-boolean" << argLocation(1)
               << "is type " << rhs->toString() << argLocation(0) << "is type "
               << lhs->toString();
      }
      
      case BinaryOp::OpType::BitwiseXor:  
         assert(false && "We don't support bitwise xor operations");

      case BinaryOp::OpType::Subtract:
      case BinaryOp::OpType::Multiply:
      case BinaryOp::OpType::Divide:
      case BinaryOp::OpType::Modulo: {
         if(lhs->isNumeric() && rhs->isNumeric()) {
            return op.resolveResultType(
                  sema.BuildBuiltInType(ast::BuiltInType::Kind::Int));
         }
         throw diag.ReportError(loc_)
               << "invalid types for "
               << BinaryOp::OpType_to_string(op.opType(), "??")
               << " operation, operands are non-numeric" << argLocation(1)
               << "is type " << rhs->toString() << argLocation(0) << "is type "
               << lhs->toString();
      }

      case BinaryOp::OpType::InstanceOf: {
         if((lhs->isNull() || isReferenceOrArrType(lhs)) &&
            (isReferenceOrArrType(rhs)) && isValidCast(rhs, lhs)) {
            return op.resolveResultType(
                  sema.BuildBuiltInType(ast::BuiltInType::Kind::Boolean));
         }
         throw diag.ReportError(loc_)
               << "invalid types for "
               << BinaryOp::OpType_to_string(op.opType(), "??")
               << " operation, operands are null or reference types that can't "
                  "be casted"
               << argLocation(1) << "is type " << rhs->toString() << argLocation(0)
               << "is type " << lhs->toString();
      }

      default:
         assert(false && "invalid binary operation");
   }
}

Type const* ExprTypeResolver::evalUnaryOp(UnaryOp& op, const Type* rhs) const {
   if(op.resultType()) return op.resultType();
   switch(op.opType()) {
      case UnaryOp::OpType::Plus:
      case UnaryOp::OpType::Minus:
      case UnaryOp::OpType::BitwiseNot:
         if(rhs->isNumeric()) {
            return op.resolveResultType(
                  sema.BuildBuiltInType(ast::BuiltInType::Kind::Int));
         }
         throw diag.ReportError(loc_)
               << "invalid type for unary "
               << UnaryOp::OpType_to_string(op.opType(), "??") << " non-numeric"
               << argLocation(0) << "is type " << rhs->toString();
      case UnaryOp::OpType::Not:
         if(rhs->isBoolean()) {
            return op.resolveResultType(
                  sema.BuildBuiltInType(ast::BuiltInType::Kind::Boolean));
         }
         throw diag.ReportError(loc_)
               << "invalid type for unary not, non-boolean" << argLocation(0)
               << "is type " << rhs->toString();
      default:
         assert(false && "invalid unary operation");
   }
}

Type const* ExprTypeResolver::evalMemberAccess(DotOp& op, const Type*,
                                               const Type* field) const {
   if(op.resultType()) return op.resultType();
   return op.resolveResultType(field);
}

Type const* ExprTypeResolver::evalMethodCall(MethodOp& op, const Type* method,
                                             const op_array& args) const {
   if(op.resultType()) return op.resultType();
   auto methodType = dynamic_cast<const MethodType*>(method);
   assert(methodType && "Not a method type");

   auto methodParams = methodType->paramTypes();
   assert(methodParams.size() == args.size() &&
          "Method params and args size mismatch");

   size_t i = 0;
   for(auto arg : args | std::views::reverse) {
      if(!isAssignableTo(methodParams[i], arg)) {
         throw diag.ReportError(loc_)
               << "invalid argument type for method call" << argLocation(i + 1)
               << "expected " << methodParams[i]->toString() << " but got "
               << arg->toString();
      }
      i++;
   }

   // Return nullptr if the method has no return type
   return op.resolveResultType(methodType->returnType());
}

Type const* ExprTypeResolver::evalNewObject(NewOp& op, const Type* object,
                                            const op_array& args) const {
   if(op.resultType()) return op.resultType();
   auto constructor = dynamic_cast<const MethodType*>(object);
   assert(constructor && "Not a method type");

   auto constructorParams = constructor->paramTypes();
   assert(constructorParams.size() == args.size() &&
          "Constructor params and args size mismatch");
   size_t i = 0;
   for(auto arg : args | std::views::reverse) {
      if(!isAssignableTo(constructorParams[i], arg)) {
         throw diag.ReportError(loc_)
               << "invalid argument type for constructor call"
               << argLocation(i + 1) << "expected "
               << constructorParams[i]->toString() << " but got "
               << arg->toString();
      }
      i++;
   }

   return op.resolveResultType(constructor->returnType());
}

Type const* ExprTypeResolver::evalNewArray(NewArrayOp& op, const Type* array,
                                           const Type* size) const {
   if(op.resultType()) return op.resultType();
   if(!size->isNumeric()) {
      throw diag.ReportError(loc_)
            << "invalid type for array size, non-numeric" << argLocation(0)
            << "is type " << size->toString();
   }
   return op.resolveResultType(array);
}

Type const* ExprTypeResolver::evalArrayAccess(ArrayAccessOp& op, const Type* array,
                                              const Type* index) const {
   if(op.resultType()) return op.resultType();
   auto arrayType = dynamic_cast<const ArrayType*>(array);
   if(!arrayType) {
      throw diag.ReportError(loc_)
            << "invalid type for array access, non-array" << argLocation(0)
            << "is type " << array->toString();
   }
   if(!index->isNumeric()) {
      throw diag.ReportError(loc_)
            << "invalid type for array index, non-numeric" << argLocation(0)
            << "is type " << index->toString();
   }

   return op.resolveResultType(arrayType->getElementType());
}

Type const* ExprTypeResolver::evalCast(CastOp& op, const Type* type,
                                       const Type* value) const {
   if(op.resultType()) return op.resultType();
   if(!isValidCast(value, type)) {
      throw diag.ReportError(loc_)
            << "invalid cast from " << value->toString() << " to "
            << type->toString() << argLocation(1) << "is type "
            << value->toString() << argLocation(0) << "is type "
            << type->toString();
   }

   return op.resolveResultType(type);
}

} // namespace semantic
