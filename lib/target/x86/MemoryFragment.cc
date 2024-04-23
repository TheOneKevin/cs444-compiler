#include "Target.h"
#include "mc/MCFunction.h"

namespace target::x86 {

bool MatchMemoryPatternFragment(mc::MatchOptions& options, mc::InstSelectNode*&) {
   auto [TD, def, ops, nodesToDelete, node] = options;
   if(TD.isRegisterClass(static_cast<unsigned>(x86RegClass::GPR64),
                         node->type())) {
      return true;
   }
   return false;
}

} // namespace target::x86
