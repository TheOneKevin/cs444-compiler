#include <ostream>
#include <string>

#include "ast/AST.h"

namespace ast {

using std::ostream;
using std::string;

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
   int id = dp.id();
   dp.startTLabel(id);
   dp.printTableSingleRow("VarDecl");
   dp.printTableDoubleRow("type", type()->toString());
   dp.printTableDoubleRow("name", name());
   dp.printTableDoubleRow("init", "", {}, {"port", "init"});
   dp.endTLabel();
   if(hasInit()) dp.printConnection(id, ":init", init()->printDotNode(dp));
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
   int id = dp.id();
   dp.startTLabel(id);
   dp.printTableSingleRow("FieldDecl");
   dp.printTableDoubleRow("modifiers", modifiers.toString());
   dp.printTableDoubleRow("type", type()->toString());
   dp.printTableDoubleRow("name", name());
   dp.printTableDoubleRow("init", "", {}, {"port", "init"});
   dp.endTLabel();
   if(hasInit()) dp.printConnection(id, ":init", init()->printDotNode(dp));
   return id;
}

string_view FieldDecl::getCanonicalName() const {
   return "";
}

} // namespace ast
