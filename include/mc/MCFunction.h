#pragma once

#include "mc/InstSelectNode.h"
#include "tir/Context.h"
#include "utils/BumpAllocator.h"
#include "utils/DotPrinter.h"

namespace mc {

class ISelDAGBuilder;

class MCFunction final {
   friend class ISelDAGBuilder;

public:
   std::ostream& printDot(std::ostream& os) const {
      utils::DotPrinter dp{os};
      std::unordered_set<InstSelectNode const*> visited;
      dp.startGraph();
      dp.print("compound=true;");
      // First, print all the nodes and connections
      for(auto* graph : graphs_) dp.id(graph);
      for(auto* graph : graphs_) {
         dp.startSubgraph(dp.getId(graph));
         graph->printDotNode(dp, visited);
         dp.endSubgraph();
      }
      // Next. print the edges that cross the subgraphs
      for(auto* graph : graphs_) {
         for(auto* pred : graph->users()) {
            // FIXME(kevin): This seems like a bad bug, why does this occur?
            if(dp.getId(pred) == -1) continue;
            auto from = dp.getId(pred);
            auto to = dp.getId(graph);
            dp.printConnection(from, to, {"color", "blue", "style", "dashed"});
         }
      }
      dp.endGraph();
      return os;
   }

private:
   MCFunction(BumpAllocator& alloc, tir::TargetInfo const& TI)
         : TI_{TI}, graphs_{alloc} {}

private:
   tir::TargetInfo const& TI_;
   std::pmr::vector<InstSelectNode*> graphs_;
};

} // namespace mc
