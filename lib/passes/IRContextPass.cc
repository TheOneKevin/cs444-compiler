#include "passes/IRContextPass.h"

#include "target/TargetDesc.h"
#include "tir/CompilationUnit.h"
#include "utils/PassManager.h"

using namespace utils;

Pass& NewIRContextPass(PassManager& PM, tir::CompilationUnit& CU,
                       target::TargetDesc const& TD) {
   return PM.AddPass<IRContextPass>(CU, TD);
}
