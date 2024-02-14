#include "AstNode.h"

namespace ast {

void Modifiers::set(parsetree::Modifier target) {
   switch(target.get_type()) {
      case parsetree::Modifier::Type::Public:
         modifiers = (uint8_t) Type::Public;
         break;
      case parsetree::Modifier::Type::Protected:
         modifiers = (uint8_t) Type::Protected;
         break;
      case parsetree::Modifier::Type::Static:
         modifiers = (uint8_t) Type::Static;
         break;
      case parsetree::Modifier::Type::Final:
         modifiers = (uint8_t) Type::Final;
         break;
      case parsetree::Modifier::Type::Abstract:
         modifiers = (uint8_t) Type::Abstract;
         break;
      case parsetree::Modifier::Type::Native:
         modifiers = (uint8_t) Type::Native;
         break;
      default:
         assert(false && "Unknown modifier type");
   }
   modifierLocations[(int)modifiers] = target.location();
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
