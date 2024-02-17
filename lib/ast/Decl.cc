#include <ostream>
#include <string>

#include "ast/AST.h"
#include "semantic/NameResolver.h"

namespace ast {

using std::ostream;
using std::string;

void AstNode::dump() const {
   print(std::cout, 0);
}

// VarDecl /////////////////////////////////////////////////////////////////////

ostream& VarDecl::print(ostream& os, int indentation) const {
   auto i1 = indent(indentation);
   os << i1 << "VarDecl {\n"
      << i1 << "  type: " << type()->toString() << "\n"
      << i1 << "  name: " << name() << "\n";
   if(init_) {
      init_->print(os, indentation + 1);
   }
   os << i1 << "}\n";
   return os;
}

int VarDecl::printDotNode(DotPrinter& dp) const {
   int id = dp.id(this);
   dp.startTLabel(id);
   dp.printTableSingleRow("VarDecl", {"bgcolor", "lightblue"});
   dp.printTableDoubleRow("type", type()->toString());
   dp.printTableDoubleRow("name", name());
   std::ostringstream expr;
   if(init_) init_->print(expr, -1);
   dp.printTableDoubleRow("init", expr.str(), {"port", "init"}, {"balign", "left"});
   dp.endTLabel();
   return id;
}

// FieldDecl ///////////////////////////////////////////////////////////////////

ostream& FieldDecl::print(ostream& os, int indentation) const {
   auto i1 = indent(indentation);
   os << i1 << "FieldDecl {\n"
      << i1 << "  modifiers: " << modifiers.toString() << "\n"
      << i1 << "  type: " << type()->toString() << "\n"
      << i1 << "  name: " << name() << "\n";
   if(hasInit()) {
      init()->print(os, indentation + 1);
   }
   os << i1 << "}\n";
   return os;
}

int FieldDecl::printDotNode(DotPrinter& dp) const {
   int id = dp.id(this);
   dp.startTLabel(id);
   dp.printTableSingleRow("FieldDecl", {"bgcolor", "lightblue"});
   dp.printTableDoubleRow("modifiers", modifiers.toString());
   dp.printTableDoubleRow("type", type()->toString());
   dp.printTableDoubleRow("name", name());
   std::ostringstream expr;
   if(hasInit()) init()->print(expr, -1);
   dp.printTableDoubleRow("init", expr.str(), {"port", "init"}, {"balign", "left"});
   dp.endTLabel();
   return id;
}

} // namespace ast
