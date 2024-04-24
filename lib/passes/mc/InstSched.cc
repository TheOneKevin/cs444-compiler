#include <queue>

#include "mc/InstSelectNode.h"
#include "mc/MCFunction.h"
#include "passes/IRContextPass.h"
#include "utils/PassManager.h"

using std::string_view;
using utils::Pass;
using utils::PassManager;
using namespace mc;
using ISN = mc::InstSelectNode;

/* ===--------------------------------------------------------------------=== */
// InstSched pass
/* ===--------------------------------------------------------------------=== */

class InstSched : public Pass {
public:
   InstSched(PassManager& PM) noexcept : Pass(PM) {}
   string_view Name() const override { return "isched"; }
   string_view Desc() const override {
      return "Instruction scheduling on MIR DAG";
   }
   void Init() override {
      dumpDot = PM().PO().GetExistingOption("--debug-mc")->count();
   }
   void Run() override;

private:
   // Run on the function
   void runOnFunction(MCFunction* F);
   // Recursively build the adjacency list of the DAG
   void buildAdjacencyList(ISN* node);
   // Topological sort the DAG
   InstSelectNode* topoSort();

   void computeDependencies() override {
      ComputeDependency(GetPass<IRContextPass>());
      ComputeDependency(GetPass("isel"));
   }

private:
   std::unordered_map<InstSelectNode*, std::vector<InstSelectNode*>> adj;
   bool dumpDot;
};

/* ===--------------------------------------------------------------------=== */
// Main scheduling algorithm
/* ===--------------------------------------------------------------------=== */

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

void InstSched::Run() {
   auto& IRCP = GetPass<IRContextPass>();
   for(auto* F : IRCP.CU().functions()) {
      if(!F->hasBody()) continue;
      auto* MCF = IRCP.FindMIRFunction(F);
      runOnFunction(MCF);
      if(dumpDot) {
         std::ofstream out{std::string{F->name()} + ".dag.isched.dot"};
         MCF->printDot(out);
      }
   }
}

void InstSched::runOnFunction(MCFunction* MCF) {
   std::unordered_set<Edge> edgesRemoved{};
   for(auto& mbb : MCF->subgraphs()) {
      assert(mbb.root->kind() == NodeKind::Entry && "Graph root is not Entry node");
      adj.clear();
      buildAdjacencyList(mbb.root);
      mbb.entry = topoSort();
   }
}

/* ===--------------------------------------------------------------------=== */
// Helper functions
/* ===--------------------------------------------------------------------=== */

void InstSched::buildAdjacencyList(ISN* node) {
   for(auto* child : node->childNodes()) {
      if(!child || child->arity() == 0) continue;
      adj[node].push_back(cast<ISN>(child));
   }
   for(unsigned i = node->numChildren(); i > 0; i--) {
      auto child = node->getChild(i - 1);
      if(!child || child->arity() == 0) continue;
      buildAdjacencyList(child);
   }
}

InstSelectNode* InstSched::topoSort() {
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
   int curTopoIdx = 0;
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
   assert(!topologicalOrder.empty());
   InstSelectNode* current = topologicalOrder.front();
   for(size_t i = 1; i < topologicalOrder.size(); ++i) {
      // Chain the nodes in reverse topological order
      current->insertAfter(topologicalOrder[i]);
      current = topologicalOrder[i];
      // Update the live ranges of each node
      for(auto& [user, index] : current->uses()) {
         // Skip chain nodes
         if(index >= user->arity() || user->topoIdx() < 0) continue;
         // Updates uses, assuming use does not escape the bb
         current->updateLiveRange(user->topoIdx());
      }
   }
   return current;
}

REGISTER_PASS(InstSched)
