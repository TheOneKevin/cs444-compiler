#include "mc/MCFunction.h"

#include <queue>
#include <unordered_set>

#include "mc/InstSelectNode.h"
#include "mc/MCPatterns.h"
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
   for(auto* graph : graphs_) {
      assert(graph->kind() == NodeKind::Entry && "Graph root is not Entry node");
      std::unordered_map<InstSelectNode*, std::vector<InstSelectNode*>> adj;
      // Build the adjacency list
      graph->buildAdjacencyList(adj);
      // Topological sort the graph
      topoSort(adj);
   }
   // Set mirRoot_ to the start?
}

void MCFunction::topoSort(
      std::unordered_map<InstSelectNode*, std::vector<InstSelectNode*>> adj) {
   std::unordered_map<InstSelectNode*, int> inDegree;
   std::queue<InstSelectNode*> queue;
   std::vector<InstSelectNode*> topologicalOrder;
   // Initialize inDegree for all nodes
   for(auto& [v, children] : adj) {
      if(inDegree.find(v) == inDegree.end())
         inDegree[v] = 0; // Ensure all nodes are in inDegree map
      for(InstSelectNode* node : children) inDegree[node]++;
   }
   // Enqueue all nodes with in-degree of 0
   for(auto& [v, deg] : inDegree)
      if(deg == 0) queue.push(v);
   // Process the graph
   while(!queue.empty()) {
      InstSelectNode* current = queue.front();
      queue.pop();
      current->setTopoIdx(curTopoIdx++);
      topologicalOrder.push_back(current);
      for(auto* neighbor : adj[current]) {
         inDegree[neighbor]--;
         if(inDegree[neighbor] == 0) queue.push(neighbor);
      }
   }
   if(topologicalOrder.empty()) return;
   InstSelectNode* current = topologicalOrder.front();
   for(size_t i = 1; i < topologicalOrder.size(); ++i) {
      current->link(topologicalOrder[i]);
      current = topologicalOrder[i];
   }
   return;
}

void MCFunction::selectInstructions() {
   std::queue<InstSelectNode*> worklist;
   std::unordered_set<InstSelectNode*> visited;
   for(auto* root : graphs_) worklist.emplace(root);
   while(!worklist.empty()) {
      auto* root = worklist.front();
      worklist.pop();
      if(visited.count(root)) continue;
      visited.emplace(root);
      if(root->arity() > 0 && root->kind() != NodeKind::MachineInstr) {
         root = matchAndReplace(root);
      }
      for(auto* child : root->childNodes()) {
         if(child) {
            worklist.emplace(child);
         }
      }
   }
}

InstSelectNode* MCFunction::matchAndReplace(InstSelectNode* root) const {
   std::vector<InstSelectNode*> operands;
   std::vector<InstSelectNode*> nodesToDelete;
   for(auto* def : TD.getMCPatterns().getPatternFor(root->kind())) {
      for(auto* pat : def->patterns()) {
         operands.resize(def->adjustOperandIndex(def->numInputs(), TD), nullptr);
         nodesToDelete.clear();
         if(pat->matches(
                  mc::MatchOptions{TD, def, operands, nodesToDelete, root})) {
            // Now we build the new node
            return root->selectPattern(alloc_, def, operands, nodesToDelete);
         }
      }
   }
   return root;
}

} // namespace mc
