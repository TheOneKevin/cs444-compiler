#include "CompilerPasses.h"
#include "IRPasses.h"
#include "codegen/CodeGen.h"
#include "diagnostics/Diagnostics.h"
#include "utils/PassManager.h"

using std::string_view;
using utils::Pass;
using utils::PassManager;
using DE = diagnostics::DiagnosticEngine;

namespace {

class Codegen final : public Pass {
public:
   Codegen(PassManager& PM) noexcept : Pass(PM) {}
   void Run() override {
      auto& NR = GetPass<passes::joos1::NameResolver>();
      auto& HC = GetPass<passes::joos1::HierarchyChecker>();
      auto* LU = GetPass<passes::joos1::Linker>().LinkingUnit();
      auto& CU = GetPass<passes::IRContext>().CU();
      codegen::CodeGenerator CG{CU.ctx(), CU, NR.Resolver(), HC.Checker()};
      CG.run(LU);
   }
   string_view Name() const override { return "codegen-tir"; }
   string_view Desc() const override { return "TIR code generation"; }

private:
   void ComputeDependencies() override {
      AddDependency(GetPass<passes::joos1::NameResolver>());
      AddDependency(GetPass<passes::joos1::HierarchyChecker>());
      AddDependency(GetPass<passes::joos1::Linker>());
      AddDependency(GetPass<passes::IRContext>());
   }
};

}

REGISTER_PASS(Codegen);
