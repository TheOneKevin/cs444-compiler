#include <string_view>

#include "CompilerPasses.h"
#include "utils/PassManager.h"

using std::string_view;
using utils::Pass;
using utils::PassManager;

using namespace joos1;

class TypeCheckerPass final : public Pass {
public:
   TypeCheckerPass(PassManager& PM) noexcept : Pass(PM) {}
   string_view Name() const override { return "sema-type"; }
   string_view Desc() const override { return "Type Checking"; }
   void Run() override {
      // auto lu = GetPass<LinkerPass>().LinkingUnit();
      // TODO(kevin):
   }

private:
   void computeDependencies() override {
      ComputeDependency(GetPass<AstContextPass>());
      ComputeDependency(GetPass<LinkerPass>());
   }
};

REGISTER_PASS(TypeCheckerPass);
