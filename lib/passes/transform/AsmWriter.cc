#include "../IRContextPass.h"
#include "tir/BasicBlock.h"
#include "tir/Constant.h"
#include "utils/PassManager.h"

#include <unordered_map>

using std::string_view;
using utils::Pass;
using utils::PassManager;

class AsmWriter final : public Pass {
public:
   AsmWriter(PassManager& PM) noexcept : Pass(PM) {}
   void Run() override {
      tir::CompilationUnit& CU = GetPass<IRContextPass>().CU();
      for(auto* F : CU.functions()) {
         if(F->hasBody())
            emitFunction(F);
      }
   }
   string_view Name() const override { return "asmwriter"; }
   string_view Desc() const override { return "Emit Assembly"; }

private:
   void emitFunction(tir::Function* F);
   void emitBasicBlock(tir::BasicBlock* BB);

   void computeDependencies() override {
      ComputeDependency(GetPass<IRContextPass>());
   }

   // Map is reset per function
   std::unordered_map<tir::Value*, int> valueStackMap;
};

REGISTER_PASS(AsmWriter);

void AsmWriter::emitFunction(tir::Function* F) {
   // 1. Map all values to a stack slot
   
   // 2. Emit the function prologue

   // 3. Emit the instructions in each basic block
}

void AsmWriter::emitBasicBlock(tir::BasicBlock* BB) {
   // 1. Emit the label for the basic block

   // 2. Emit the instructions in the basic block
}
