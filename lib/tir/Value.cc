#include "tir/TIR.h"

namespace tir {

void Value::dump() const { print(std::cerr) << "\n"; }

void Value::replaceAllUsesWith(Value* newValue) {
   for(auto user : users_) {
      user->replaceChild(user->numChildren(), newValue);
   }
}

} // namespace tir
