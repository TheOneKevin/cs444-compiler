#include <unordered_map>

#include "mc/InstSelectNode.h"
#include "target/TargetDesc.h"
#include "target/TargetInfo.h"
#include "tir/CompilationUnit.h"
#include "tir/Constant.h"
#include "utils/BumpAllocator.h"
#include "utils/PassManager.h"

class IRContextPass final : public utils::Pass {
public:
   IRContextPass(utils::PassManager& PM, tir::CompilationUnit& CU,
                 target::TargetDesc const& TD) noexcept
         : Pass(PM), CU_{CU}, TD_{TD} {}
   std::string_view Desc() const override { return "TIR + MIR Context Lifetime"; }
   void Init() override {}
   void Run() override {}
   tir::CompilationUnit& CU() { return CU_; }
   tir::CompilationUnit const& CU() const { return CU_; }
   target::TargetDesc const& TD() const { return TD_; }
   target::TargetInfo const& TI() const { return CU_.ctx().TI(); }
   BumpAllocator& Alloc() const {
      // FIXME(kevin): Should this really be taking the context heap?
      return CU_.ctx().alloc();
   }
   mc::MCFunction* FindMIRFunction(tir::Function const* fn) const {
      auto it = mirFuncMap_.find(fn);
      return it != mirFuncMap_.end() ? it->second : nullptr;
   }
   void AddMIRFunction(tir::Function const* fn, mc::MCFunction* mirFn) {
      mirFuncMap_.emplace(fn, mirFn);
   }
   ~IRContextPass() override {}

private:
   void computeDependencies() override {}
   tir::CompilationUnit& CU_;
   target::TargetDesc const& TD_;
   std::unordered_map<tir::Function const*, mc::MCFunction*> mirFuncMap_;
};
