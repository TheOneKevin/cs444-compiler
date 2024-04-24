#include "mc/MCFunction.h"

#include <unordered_set>

#include "mc/InstSelectNode.h"
#include "utils/DotPrinter.h"

namespace mc {

std::ostream& MCFunction::printDot(std::ostream& os) const {
   utils::DotPrinter dp{os, "20"};
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
         if(pred->kind() != NodeKind::BasicBlock) continue;
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

} // namespace mc
