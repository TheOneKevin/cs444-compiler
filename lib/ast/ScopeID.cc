#include <sstream>

#include "ast/AST.h"

namespace ast {

std::string ScopeID::toString() const {
   std::ostringstream oss;
   print(oss);
   return oss.str();
}

std::ostream& ScopeID::print(std::ostream& os) const {
   if(parent_) {
      parent_->print(os);
      os << ".";
   }
   os << pos_;
   return os;
}

void ScopeID::dump() const { print(std::cerr) << std::endl; }

} // namespace ast