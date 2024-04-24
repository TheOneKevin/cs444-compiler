#pragma once

#include "mc/InstSelectNode.h"
#include "target/Target.h"
#include "utils/BumpAllocator.h"

namespace mc {

class MCFunction final {
public:
   MCFunction(BumpAllocator& alloc, target::TargetInfo const& TI,
              target::TargetDesc const& TD)
         : alloc_{alloc}, TI_{TI}, TD_{TD}, graphs_{alloc} {}

   /**
    * @brief Runs a reversed topological sort on the basic block DAG
    * subgraphs and emits a single DAG node as the machine IR root.
    */
   void scheduleMIR();
   /// @brief Prints the DAG as a collection of subgraphs in DOT format
   std::ostream& printDot(std::ostream& os) const;
   /// @brief Gets the allocator
   BumpAllocator& alloc() const { return alloc_; }
   /// @brief Gets the target info
   auto const& TI() const { return TI_; }
   /// @brief Gets the target desc
   auto const& TD() const { return TD_; }
   /// @brief Adds a subgraph (basic block) to the function
   void addSubgraph(InstSelectNode* graph) {
      assert(graph->kind() == NodeKind::Entry && "Subgraph must be an entry node");
      graphs_.push_back(graph);
   }
   /// @brief Iterate the subgraphs of the function
   auto subgraphs() const { return std::views::all(graphs_); }

private:
   void topoSort(
         std::unordered_map<InstSelectNode*, std::vector<InstSelectNode*>> adj);

private:
   BumpAllocator& alloc_;
   target::TargetInfo const& TI_;
   target::TargetDesc const& TD_;
   std::pmr::vector<InstSelectNode*> graphs_;
   InstSelectNode* mirRoot_;
   int curTopoIdx = 0;
};

} // namespace mc
