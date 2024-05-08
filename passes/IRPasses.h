#pragma once

#include <unordered_map>

#include "AllPasses.h"
#include "mc/InstSelectNode.h"
#include "target/TargetDesc.h"
#include "target/TargetInfo.h"
#include "tir/CompilationUnit.h"
#include "tir/Constant.h"
#include "utils/BumpAllocator.h"
#include "utils/PassManager.h"

namespace passes {

class IRContext final : public utils::Pass {
public:
   IRContext(utils::PassManager& PM) noexcept;
   std::string_view Name() const override { return "ir-context"; }
   std::string_view Desc() const override { return "TIR + MIR Context Lifetime"; }
   void Init() override {}
   void Run() override {}
   tir::CompilationUnit& CU() { return CU_; }
   tir::CompilationUnit const& CU() const { return CU_; }
   target::TargetDesc const& TD() const { return TD_; }
   target::TargetInfo const& TI() const { return CU_.ctx().TI(); }
   BumpAllocator& Alloc() const { return alloc_; }
   mc::MCFunction* FindMIRFunction(tir::Function const* fn) const {
      auto it = mirFuncMap_.find(fn);
      return it != mirFuncMap_.end() ? it->second : nullptr;
   }
   void AddMIRFunction(tir::Function const* fn, mc::MCFunction* mirFn) {
      mirFuncMap_.emplace(fn, mirFn);
   }
   ~IRContext() override {}

private:
   void ComputeDependencies() override {}

private:
   utils::CustomBufferResource heap_;
   mutable BumpAllocator alloc_;
   target::TargetInfo& TI_;
   target::TargetDesc& TD_;
   tir::Context ctx_;
   tir::CompilationUnit CU_;
   std::unordered_map<tir::Function const*, mc::MCFunction*> mirFuncMap_;
};

class BasicBlock : public utils::Pass {
public:
   BasicBlock(utils::PassManager& PM) noexcept : Pass(PM) {}
   void Run() override final;
   int Tag() const override final {
      return static_cast<int>(PassTag::BasicBlockPass);
   }

protected:
   virtual void runOnBasicBlock(tir::BasicBlock* bb) = 0;
   void ComputeDependencies() override final {
      AddDependency(GetPass<IRContext>());
      ComputeMoreDependencies();
   }
   virtual void ComputeMoreDependencies() {}
};

class Function : public utils::Pass {
public:
   Function(utils::PassManager& PM) noexcept : Pass(PM) {}
   void Run() override final;
   int Tag() const override final {
      return static_cast<int>(PassTag::FunctionPass);
   }

protected:
   virtual void runOnFunction(tir::Function* fn) = 0;
   void ComputeDependencies() override final {
      AddDependency(GetPass<IRContext>());
      ComputeMoreDependencies();
   }
   virtual void ComputeMoreDependencies() {}
};

class CompilationUnit : public utils::Pass {
public:
   CompilationUnit(utils::PassManager& PM) noexcept : Pass(PM) {}
   void Run() override final;
   int Tag() const override final {
      return static_cast<int>(PassTag::CompilationUnitPass);
   }

protected:
   virtual void runOnCompilationUnit(tir::CompilationUnit* cu) = 0;
   void ComputeDependencies() override final {
      AddDependency(GetPass<IRContext>());
      ComputeMoreDependencies();
   }
   virtual void ComputeMoreDependencies() {}
};

} // namespace passes
