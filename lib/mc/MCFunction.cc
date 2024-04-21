#include "mc/MCFunction.h"

#include <unordered_set>

#include "mc/InstSelectNode.h"
#include "utils/DotPrinter.h"

struct Edge {
   mc::InstSelectNode* from = nullptr;
   mc::InstSelectNode* to = nullptr;
};

template <>
struct std::hash<Edge> {
   using T = mc::InstSelectNode*;
   std::size_t operator()(const Edge& e) const {
      return std::hash<T>{}(e.from) ^ std::hash<T>{}(e.to);
   }
};

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

void MCFunction::scheduleMIR() {
   std::unordered_set<Edge> edgesRemoved{};
   /**
    * 1. Topological sort the DAGs & while topo-sorting:
    *    a) Build the MIR linked-list in reverse using node.insertBefore()
    *    b) Set the topoIdx of each node
    *    c) Update the live range of each node too
    * 2. The root is the last node
    * 3. Remove all the chains from each node
    */
   mirRoot_ = nullptr;
}

void MCFunction::selectInstructions() {
   for(auto* root : graphs_) {
      TD.getMCPatterns().matchAndReplace(root);
   }
}

} // namespace mc
