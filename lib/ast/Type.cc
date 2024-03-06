#include "ast/AST.h"

namespace ast {

MethodType::MethodType(BumpAllocator& alloc, MethodDecl const* method)
      : Type{SourceRange{}}, returnType_{nullptr}, paramTypes_{alloc} {
   returnType_ = method->returnTy().type;
   for(auto param : method->parameters()) {
      paramTypes_.push_back(param->type());
   }
}

std::ostream& BuiltInType::print(std::ostream& os, int indentation) const {
   indent(indentation);
   return os << "BasicTy:" << toString();
}

std::ostream& ReferenceType::print(std::ostream& os, int indentation) const {
   indent(indentation);
   return os << "RefTy:" << toString();
}

std::ostream& UnresolvedType::print(std::ostream& os, int indentation) const {
   indent(indentation);
   return os << "UnresTy:" << toString();
}

std::ostream& ArrayType::print(std::ostream& os, int indentation) const {
   indent(indentation);
   return getElementType()->print(os) << "[]";
}

std::ostream& MethodType::print(std::ostream& os, int indentation) const {
   indent(indentation);
   returnType_->print(os) << " (";
   for(auto param : paramTypes_) param->print(os) << ", ";
   os << ")";
   return os;
}

} // namespace ast