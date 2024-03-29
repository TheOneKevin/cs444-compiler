#include "../IRContextPass.h"
#include "tir/BasicBlock.h"
#include "utils/PassManager.h"

using std::string_view;
using utils::Pass;
using utils::PassManager;

class GlobalDCE final : public Pass {
public:
   GlobalDCE(PassManager& PM) noexcept : Pass(PM) {}
   void Run() override {
      tir::CompilationUnit& CU = GetPass<IRContextPass>().CU();
      for(auto func : CU.global_objects()) {
         
      }
   }
   string_view Name() const override { return "globaldce"; }
   string_view Desc() const override { return "Global dead code elimination"; }

private:
   void computeDependencies() override {
      ComputeDependency(GetPass<IRContextPass>());
   }
};

REGISTER_PASS(GlobalDCE);
