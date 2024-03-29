#include "tir/CompilationUnit.h"
#include "utils/PassManager.h"

class IRContextPass final : public utils::Pass {
public:
   IRContextPass(utils::PassManager& PM, tir::CompilationUnit& CU) noexcept
         : Pass(PM), cu{CU} {}
   std::string_view Desc() const override { return "TIR Context Lifetime"; }
   void Init() override {}
   void Run() override {}
   tir::CompilationUnit& CU() { return cu; }
   ~IRContextPass() override {}

private:
   void computeDependencies() override {}
   tir::CompilationUnit& cu;
};
