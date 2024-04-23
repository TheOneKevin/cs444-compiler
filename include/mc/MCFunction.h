#pragma once

#include "mc/InstSelectNode.h"
#include "target/Target.h"
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
   /**
    * @brief Performs instruction selection on the machine IR root node
    * for each basic block DAG subgraph.
    */
   void selectInstructions();
   BumpAllocator& alloc() const { return alloc_; }
   
   void topoSort(std::unordered_map<InstSelectNode*, std::vector<InstSelectNode*>> adj);


private:
   MCFunction(BumpAllocator& alloc, target::TargetInfo const& TI,
              target::TargetDesc const& TD)
         : alloc_{alloc}, TI{TI}, TD{TD}, graphs_{alloc} {}

   /**
    * @brief Uses maximal munch to match the pattern starting from the root
    * node. Returns constructed new node with the matched pattern and linked
    * parameters. Will destroy the matched node.
    *
    * @param root The node to begin matching at
    * @return mc::InstSelectNode* The new node with the matched pattern
    */
   InstSelectNode* matchAndReplace(InstSelectNode* root) const;

private:
   BumpAllocator& alloc_;
   target::TargetInfo const& TI;
   target::TargetDesc const& TD;
   std::pmr::vector<InstSelectNode*> graphs_;
   InstSelectNode* mirRoot_;
   int curTopoIdx = 0;
};

} // namespace mc
