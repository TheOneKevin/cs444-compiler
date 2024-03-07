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

bool ScopeID::canView(ScopeID const* other) const {
   assert(other != nullptr && "Can't view the null scope");
   // If under same scope, we can view other iff we are later position
   if(this->parent_ == other->parent_) {
      return this->pos_ >= other->pos_;
   }
   // If under different scope, check if other is visible from parent
   if(this->parent_) {
      return this->parent_->canView(other);
   }
   // If we're the topmost scope, then other is a child we cannot see.
   return false;
}

} // namespace ast