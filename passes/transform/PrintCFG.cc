#include "../IRPasses.h"
#include "tir/Constant.h"
#include "utils/PassManager.h"

using std::string_view;
using utils::PassManager;

namespace {

class PrintCFG final : public passes::CompilationUnit {
public:
   PrintCFG(PassManager& PM) noexcept : passes::CompilationUnit(PM) {}
   void runOnCompilationUnit(tir::CompilationUnit* CU) override {
      for(auto fn : CU->functions()) {
         if(!fn->hasBody()) continue;
         std::ofstream file(std::to_string(number) + "." +
                            std::string{fn->name()} + ".dot");
         fn->printDot(file);
         file.close();
      }
      number++;
   }
   string_view Name() const override { return "printcfg"; }
   string_view Desc() const override { return "Dump CFG DOT (per function)"; }

private:
   int number = 0;
};

} // namespace

REGISTER_PASS(PrintCFG);
