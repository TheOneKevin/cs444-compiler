#include "CompilerPasses.h"
#include "semantic/ConstantTypeResolver.h"
#include "semantic/DataflowAnalysis.h"
#include "utils/PassManager.h"

using namespace joos1;
using namespace semantic;
using std::string_view;
using utils::Pass;
using utils::PassManager;

class DFAPass final : public Pass {
public:
   DFAPass(PassManager& PM) noexcept : Pass(PM) {}
   string_view Name() const override { return "dfa"; }
   string_view Desc() const override { return "Dataflow Analysis"; }
   void Run() override {
      auto LU = GetPass<LinkerPass>().LinkingUnit();
      auto& Sema = GetPass<AstContextPass>().Sema();
      ConstantTypeResolver CTR{NewHeap()};
      DataflowAnalysis DFA{PM().Diag(), NewHeap(), Sema, LU};
      CFGBuilder builder{PM().Diag(), &CTR, NewHeap(), Sema};
      DFA.init(&builder);

      try {
         DFA.LiveVariableAnalysis();
      } catch(const diagnostics::DiagnosticBuilder&) {
         // Print the errors from diag in the next step
      }
   }

private:
   void computeDependencies() override {
      ComputeDependency(GetPass<AstContextPass>());
      ComputeDependency(GetPass<LinkerPass>());
   }
};

REGISTER_PASS(DFAPass);