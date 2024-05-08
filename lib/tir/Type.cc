#include "tir/Type.h"

#include <iostream>
#include <ostream>

#include "tir/Constant.h"

namespace tir {

static_assert(sizeof(IntegerType) == sizeof(Type));
static_assert(sizeof(FunctionType) == sizeof(Type));
static_assert(sizeof(ArrayType) == sizeof(Type));
static_assert(sizeof(StructType) == sizeof(Type));

Type* Type::getInt1Ty(Context& ctx) { return IntegerType::get(ctx, 1); }
Type* Type::getInt8Ty(Context& ctx) { return IntegerType::get(ctx, 8); }
Type* Type::getInt16Ty(Context& ctx) { return IntegerType::get(ctx, 16); }
Type* Type::getInt32Ty(Context& ctx) { return IntegerType::get(ctx, 32); }

std::ostream& Type::print(std::ostream& os) const {
   if(isIntegerType()) {
      os << "i" << static_cast<const IntegerType*>(this)->getBitWidth();
   } else if(isFunctionType()) {
      os << "function";
   } else if(isPointerType()) {
      os << "ptr";
   } else if(isArrayType()) {
      auto ty = static_cast<const ArrayType*>(this);
      os << "[" << ty->getLength() << " x " << *ty->getElementType() << "]";
   } else if(isStructType()) {
      // Get the struct name in the context
      os << "struct";
      int i = 0;
      for(auto ty : ctx_->pimpl().structTypes) {
         if(ty == this) {
            os << "." << i;
            break;
         }
         i++;
      }
   } else if(isVoidType()) {
      os << "void";
   } else {
      os << "unknown";
   }
   return os;
}

void Type::dump() const {
   print(std::cerr);
   std::cerr << std::endl;
}

std::ostream& StructType::printDetail(std::ostream& os) const {
   os << "type " << *this << " = ";
   os << "struct {";
   bool isFirst = true;
   for(auto& elem : this->getElements()) {
      if(!isFirst) os << ", ";
      isFirst = false;
      elem->print(os);
   }
   os << "}";
   return os;
}

std::ostream& operator<<(std::ostream& os, const Type& type) {
   return type.print(os);
}

Type* StructType::getIndexedType(utils::range_ref<Value*> indices) {
   int i = 0;
   Type* subTy = this;
   indices.for_each([&](auto* idx) {
      if(subTy->isStructType()) {
         subTy = cast<StructType>(subTy)->getTypeAtIndex(
               cast<ConstantInt>(idx)->zextValue());
      } else if(subTy->isArrayType()) {
         subTy = cast<ArrayType>(subTy)->getElementType();
      } else {
         assert(false && "Invalid index for GEP");
      }
      i++;
   });
   return subTy;
}

} // namespace tir
