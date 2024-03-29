#include "../IRContextPass.h"
#include "tir/Constant.h"
#include "utils/PassManager.h"

using std::string_view;
using utils::Pass;
using utils::PassManager;

class PrintCFG final : public Pass {
public:
   PrintCFG(PassManager& PM) noexcept : Pass(PM) {}
   void Run() override {
      tir::CompilationUnit& CU = GetPass<IRContextPass>().CU();
      for(auto fn : CU.functions()) {
         if(!fn->hasBody()) continue;
         std::ofstream file(std::to_string(number) + "." + std::string{fn->name()} + ".dot");
         fn->printDot(file);
         file.close();
      }
      number++;
   }
   string_view Name() const override { return "printcfg"; }
   string_view Desc() const override { return "Dump CFG DOT per Function"; }

private:
   void computeDependencies() override {
      ComputeDependency(GetPass<IRContextPass>());
   }
   int number = 0;
};

REGISTER_PASS(PrintCFG);
