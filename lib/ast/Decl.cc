#include <ostream>
#include <string>

#include "ast/AST.h"

namespace ast {

using std::ostream;
using std::string;

ostream& VarDecl::print(ostream& os, int indentation) const {
   auto i1 = indent(indentation);
   os << i1 << "VarDecl {\n"
      << i1 << "  type: " << getType()->toString() << "\n"
      << i1 << "  name: " << getName() << "\n";
   if(init) {
      init->print(os, indentation + 1);
   }
   os << i1 << "}\n";
   return os;
}

ostream& FieldDecl::print(ostream& os, int indentation) const {
   auto i1 = indent(indentation);
   os << i1 << "FieldDecl {\n"
      << i1 << "  modifiers: " << modifiers.toString() << "\n"
      << i1 << "  type: " << getType()->toString() << "\n"
      << i1 << "  name: " << getName() << "\n";
   if(hasInit()) {
      getInit()->print(os, indentation + 1);
   }
   os << i1 << "}\n";
   return os;
}

} // namespace ast
