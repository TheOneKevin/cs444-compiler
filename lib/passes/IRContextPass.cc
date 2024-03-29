#include "tir/CompilationUnit.h"
#include "utils/PassManager.h"
#include "IRContextPass.h"

using namespace utils;

Pass& NewIRContextPass(PassManager& PM, tir::CompilationUnit& cu) {
   return PM.AddPass<IRContextPass>(cu);
}
