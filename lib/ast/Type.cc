#include "ast/AST.h"

namespace ast {

MethodType::MethodType(BumpAllocator& alloc, MethodDecl const* method)
      : Type{SourceRange{}}, returnType_{nullptr}, paramTypes_{alloc} {
   returnType_ = method->returnTy().type;
   for(auto param : method->parameters()) {
      paramTypes_.push_back(param->type());
   }
}

} // namespace ast