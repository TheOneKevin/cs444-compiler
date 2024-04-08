#include "mc/InstSelectDAG.h"

#include <unordered_set>

#include "mc/MCFunction.h"
#include "tir/TIR.h"

namespace mc {

InstSelectDAG* InstSelectDAG::BuildSelectionDAG(MCFunction* parent,
                                                tir::BasicBlock& bb) {
   (void)parent;
   (void)bb;
   return nullptr;
}

} // namespace mc
