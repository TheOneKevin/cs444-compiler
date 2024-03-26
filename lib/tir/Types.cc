#include "tir/Type.h"
#include <ostream>

namespace tir {

Type* Type::getInt1Ty(Context& ctx) { return IntegerType::get(ctx, 1); }
Type* Type::getInt8Ty(Context& ctx) { return IntegerType::get(ctx, 8); }
Type* Type::getInt32Ty(Context& ctx) { return IntegerType::get(ctx, 32); }

std::ostream& Type::print(std::ostream& os) const {
   if(isIntegerType()) {
      os << "i" << static_cast<const IntegerType*>(this)->getBitWidth();
   } else if(isFunctionType()) {
      os << "function";
   } else if(isPointerType()) {
      os << "ptr*";
   } else if(isArrayType()) {
      auto ty = static_cast<const ArrayType*>(this);
      os << "[" << ty->getLength() << " x " << *ty->getElementType() << "]";
   } else if(isStructType()) {
      auto ty = static_cast<const StructType*>(this);
      os << "{";
      bool isFirst = true;
      for(auto& elem : ty->getElements()) {
         if(!isFirst)
            os << ", ";
         isFirst = false;
         elem->print(os);
      }
      os << "}";
   } else if(isVoidType()) {
      os << "void";
   } else {
      os << "unknown";
   }
   return os;
}

std::ostream& operator<<(std::ostream& os, const Type& type) {
   return type.print(os);
}

} // namespace tir
