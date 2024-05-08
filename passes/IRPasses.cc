#include "IRPasses.h"

#include "target/Target.h"
#include "target/TargetDesc.h"
#include "tir/BasicBlock.h"
#include "tir/CompilationUnit.h"
#include "utils/PassManager.h"

using namespace utils;

/* ===--------------------------------------------------------------------=== */
// TIR Pass Dispatcher Implementations
/* ===--------------------------------------------------------------------=== */

namespace {

class BBDispatcher final : public utils::PassDispatcher {
public:
   std::string_view Name() override { return "BasicBlock Dispatcher"; }
   bool CanDispatch(Pass& pass) override {
      return pass.Tag() == static_cast<int>(PassTag::BasicBlockPass);
   }
   Generator<void*> Iterate(PassManager& PM) override {
      auto& CU = PM.FindPass<passes::IRContext>().CU();
      for(auto* F : CU.functions()) {
         if(!F->hasBody()) continue;
         for(auto* BB : F->body()) {
            bb_ = BB;
            co_yield nullptr;
         }
      }
   }
   tir::BasicBlock* BB() const { return bb_; }

private:
   tir::BasicBlock* bb_;
};

class FnDispatcher final : public utils::PassDispatcher {
public:
   std::string_view Name() override { return "Function Dispatcher"; }
   bool CanDispatch(Pass& pass) override {
      return pass.Tag() == static_cast<int>(PassTag::FunctionPass);
   }
   Generator<void*> Iterate(PassManager& PM) override {
      auto& CU = PM.FindPass<passes::IRContext>().CU();
      for(auto* F : CU.functions()) {
         if(!F->hasBody()) continue;
         fn_ = F;
         co_yield nullptr;
      }
   }
   tir::Function* Fn() const { return fn_; }

private:
   tir::Function* fn_;
};

class CUDispatcher final : public utils::PassDispatcher {
public:
   std::string_view Name() override { return "CompilationUnit Dispatcher"; }
   bool CanDispatch(Pass& pass) override {
      return pass.Tag() == static_cast<int>(PassTag::CompilationUnitPass);
   }
   Generator<void*> Iterate(PassManager& PM) override {
      auto& CU = PM.FindPass<passes::IRContext>().CU();
      cu_ = &CU;
      co_yield nullptr;
   }
   tir::CompilationUnit* CU() const { return cu_; }

private:
   tir::CompilationUnit* cu_;
};

} // namespace

/* ===--------------------------------------------------------------------=== */
// TIR Pass (Base Class) Implementations
/* ===--------------------------------------------------------------------=== */

passes::IRContext::IRContext(PassManager& PM) noexcept
      : Pass(PM),
        heap_{},
        alloc_{&heap_},
        TI_{target::TargetInfo::Get<target::ArchType::X86>()},
        TD_{target::TargetDesc::Get<target::ArchType::X86>()},
        ctx_{alloc_, TI_},
        CU_{ctx_} {}

void passes::BasicBlock::Run() {
   runOnBasicBlock(GetDispatcher<BBDispatcher>()->BB());
}

void passes::Function::Run() {
   runOnFunction(GetDispatcher<FnDispatcher>()->Fn());
}

void passes::CompilationUnit::Run() {
   runOnCompilationUnit(GetDispatcher<CUDispatcher>()->CU());
}

/* ===--------------------------------------------------------------------=== */
// Register Passes
/* ===--------------------------------------------------------------------=== */

REGISTER_PASS_NS(passes, IRContext);

void AddTIRDispatchers(utils::PassManager& PM) {
   PM.AddDispatcher<::BBDispatcher>();
   PM.AddDispatcher<::FnDispatcher>();
   PM.AddDispatcher<::CUDispatcher>();
}
