#pragma once

#include "utils/BumpAllocator.h"
#include "mc/InstSelectDAG.h"
#include <vector>

namespace mc {

class MCFunction {
public:
   MCFunction(BumpAllocator& alloc) : alloc_{alloc} {}
   BumpAllocator& alloc() { return alloc_; }
   int allocVirtReg() { return highestVregIdx_++; }

private:
   BumpAllocator& alloc_;
   int highestVregIdx_ = 0;
};

} // namespace mc
