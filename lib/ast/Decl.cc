#include "ast/AST.h"
#include <ostream>
#include <string>

namespace ast {

using std::ostream;
using std::string;

ostream& VarDecl::print(ostream& os, int indentation) const {
    return os;
}

ostream& FieldDecl::print(ostream& os, int indentation) const {
    return os;
}

} // namespace ast
