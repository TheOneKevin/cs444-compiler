#include <queue>

#include "mc/InstSelectNode.h"
#include "mc/MCFunction.h"
#include "../IRContextPass.h"
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
      dumpDot = PM().GetExistingOption("--debug-mc")->count();
   }
   void Run() override;

private:
   void runOnFunction(MCFunction* F);
   void ComputeDependencies() override {
      AddDependency(GetPass<IRContextPass>());
      AddDependency(GetPass("isel"));
   }

private:
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
   
}

/* ===--------------------------------------------------------------------=== */
// Helper functions
/* ===--------------------------------------------------------------------=== */

REGISTER_PASS(InstSched)
