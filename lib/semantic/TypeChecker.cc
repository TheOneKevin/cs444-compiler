#include "TypeChecker.h"

namespace semantic {

// Field initialization
// Method body
void TypeChecker::check(ast::LinkingUnit* lu) { lu_ = lu; }

void validateExpr(ast::Expr* expr) {}

} // namespace semantic