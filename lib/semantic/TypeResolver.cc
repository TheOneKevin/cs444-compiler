#include "semantic/TypeResolver.h"

namespace semantic {

using namespace ast;
using namespace ast::exprnode;

void ExprTypeResolver::resolve() {
   // FIXME(larry,owen): Add your implementation here
   this->Evaluate(nullptr);
}

Type* ExprTypeResolver::mapValue(ExprValue const& node) const {
   // Implementation goes here
   // ...
}

Type* ExprTypeResolver::evalBinaryOp(const BinaryOp op, const Type* lhs, const Type* rhs) const {
   // Implementation goes here
   // ...
}

Type* ExprTypeResolver::evalUnaryOp(const UnaryOp op, const Type* rhs) const {
   // Implementation goes here
   // ...
}

Type* ExprTypeResolver::evalMemberAccess(const Type* lhs, const Type* field) const {
   // Implementation goes here
   // ...
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
   // Implementation goes here
   // ...
}

Type* ExprTypeResolver::evalArrayAccess(const Type* array, const Type* index) const {
   // Implementation goes here
   // ...
}

Type* ExprTypeResolver::evalCast(const Type* type, const Type* value) const {
   // Implementation goes here
   // ...
}

} // namespace semantic
