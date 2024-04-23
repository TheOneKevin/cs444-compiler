#pragma once

#include "mc/InstSelectNode.h"
#include "mc/MCTargetDesc.h"
#include "tir/Context.h"
#include "utils/BumpAllocator.h"

namespace mc {

class DAGBuilder;

class MCFunction final {
   friend class DAGBuilder;

public:
   /// @brief Prints the DAG as a collection of subgraphs in DOT format
   std::ostream& printDot(std::ostream& os) const;
   /**
    * @brief Runs a reversed topological sort on the basic block DAG
    * subgraphs and emits a single DAG node as the machine IR root.
    */
   void scheduleMIR();
   void topoSort(std::unordered_map<InstSelectNode*, std::vector<InstSelectNode*>> adj);

private:
   MCFunction(BumpAllocator& alloc, tir::TargetInfo const& TI,
              mc::MCTargetDesc const& TD)
         : TI{TI}, TD{TD}, graphs_{alloc} {}

private:
   tir::TargetInfo const& TI;
   mc::MCTargetDesc const& TD;
   std::pmr::vector<InstSelectNode*> graphs_;
   InstSelectNode* mirRoot_;
   int curTopoIdx = 0;
};

} // namespace mc
