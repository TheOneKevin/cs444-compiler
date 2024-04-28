// FIXME(kevin): This whole class is a mess

#include "Target.h"
#include "mc/InstSelectNode.h"
#include "mc/Patterns.h"

namespace target::x86 {

bool MatchMemoryPatternFragment(mc::MatchOptions& options, unsigned idx) {
   auto [TD, def, ops, nodesToDelete, node] = options;
   mc::InstSelectNode* newOps[4] = {
         nullptr, // Base
         nullptr, // Index
         nullptr, // Scale
         nullptr  // Displacement
   };
   // 1. Try to match the operands and assign newOps[]
   if(node->kind() == mc::NodeKind::FrameIndex) {
      newOps[3] = node;
   } else if(TD.isRegisterClass(static_cast<unsigned>(x86RegClass::GPR64),
                                node->type())) {
      newOps[0] = node;
   } else {
      return false;
   }
   // 2. Check if the operands are equal
   bool allNull = true;
   bool allEqual = true;
   for(int i = 0; i < 4; i++) {
      allNull = allNull && (ops[idx + i] == nullptr);
      if(ops[idx + i] == nullptr || newOps[i] == nullptr)
         allEqual = allEqual && (ops[idx + i] == newOps[i]);
      else
         allEqual = allEqual && (*ops[idx + i] == *newOps[i]);
   }
   if(!allNull && !allEqual) return false;
   // 3. If ops are all null, then it is uninitialized
   for(int i = 0; i < 4 && allNull; i++) {
      ops[idx + i] = newOps[i];
   }
   return true;
}

} // namespace target::x86
