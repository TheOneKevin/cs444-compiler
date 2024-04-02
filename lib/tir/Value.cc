#include "tir/TIR.h"

namespace tir {

void Value::dump() const { print(std::cerr) << "\n"; }

void Value::replaceAllUsesWith(Value* newValue) {
   for(auto use : uses_) {
      use.user->replaceChild(use.idx, newValue);
   }
}

} // namespace tir
