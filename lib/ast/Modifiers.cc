#include "AstNode.h"

namespace ast {

void Modifiers::set(parsetree::Modifier target) {
   Type modifier;
   switch(target.get_type()) {
      case parsetree::Modifier::Type::Public:
         modifier = Type::Public;
         break;
      case parsetree::Modifier::Type::Protected:
         modifier = Type::Protected;
         break;
      case parsetree::Modifier::Type::Static:
         modifier = Type::Static;
         break;
      case parsetree::Modifier::Type::Final:
         modifier = Type::Final;
         break;
      case parsetree::Modifier::Type::Abstract:
         modifier = Type::Abstract;
         break;
      case parsetree::Modifier::Type::Native:
         modifier = Type::Native;
         break;
      default:
         assert(false && "Unknown modifier type");
   }
   set(modifier);
   modifierLocations[(int)modifier] = target.location();
}

std::string Modifiers::toString() const {
   std::string result;
   if (test(modifiers, Type::Public)) result += "public ";
   if (test(modifiers, Type::Protected)) result += "protected ";
   if (test(modifiers, Type::Static)) result += "static ";
   if (test(modifiers, Type::Final)) result += "final ";
   if (test(modifiers, Type::Abstract)) result += "abstract ";
   if (test(modifiers, Type::Native)) result += "native ";
   return result;
}

} // namespace ast
