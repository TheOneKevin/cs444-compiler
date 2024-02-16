#pragma once

#include "ast/AST.h"

namespace semantic {

class HierarchyChecker {
public:
   HierarchyChecker(ast::LinkingUnit* lu) : lu_{lu} {}

private:
   ast::LinkingUnit* lu_;
};

} // namespace semantic
