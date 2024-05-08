#pragma once

#include "diagnostics/SourceManager.h"
#include "utils/PassManager.h"

enum class PassTag {
   None = 0,
   FrontendPass,
   BasicBlockPass,
   FunctionPass,
   CompilationUnitPass,
   MachineCodePass
};

/* ===--------------------------------------------------------------------=== */
// Front-end passes
/* ===--------------------------------------------------------------------=== */

utils::Pass& NewJoos1WParserPass(utils::PassManager& PM, SourceFile file,
                                 utils::Pass* depends);
utils::Pass& NewAstBuilderPass(utils::PassManager& PM, utils::Pass* depends);

DECLARE_PASS(HierarchyChecker);
DECLARE_PASS(AstContext);
DECLARE_PASS(Linker);
DECLARE_PASS(NameResolver);
DECLARE_PASS(PrintAST);
DECLARE_PASS(ExprResolver);
DECLARE_PASS(Dataflow);
DECLARE_PASS(Codegen);

/* ===--------------------------------------------------------------------=== */
// Optimization passes
/* ===--------------------------------------------------------------------=== */

DECLARE_PASS(IRContext);
DECLARE_PASS(SimplifyCFG);
DECLARE_PASS(GlobalDCE);
DECLARE_PASS(MemToReg);
DECLARE_PASS(PrintCFG);
DECLARE_PASS(DominatorTreeWrapper);

/* ===--------------------------------------------------------------------=== */
// Backend passes
/* ===--------------------------------------------------------------------=== */

DECLARE_PASS(InstSelect);
DECLARE_PASS(InstSched);
DECLARE_PASS(MIRBuilder);

/* ===--------------------------------------------------------------------=== */
// Functions to automatically add all these passes
/* ===--------------------------------------------------------------------=== */

void AddTIRDispatchers(utils::PassManager& PM);

/// @brief Adds all the front-end passes EXCEPT for parsing passes (per file)
static void BuildFrontEndPasses(utils::PassManager& PM) {
   NewAstContextPass(PM);
   NewLinkerPass(PM);
   NewPrintASTPass(PM);
   NewNameResolverPass(PM);
   NewHierarchyCheckerPass(PM);
   NewExprResolverPass(PM);
   NewDataflowPass(PM);
   NewCodegenPass(PM);
}

/// @brief Adds all the optimization passes EXCEPT for the context pass
static void BuildOptPasses(utils::PassManager& PM) {
   AddTIRDispatchers(PM);
   NewIRContextPass(PM);
   NewSimplifyCFGPass(PM);
   NewGlobalDCEPass(PM);
   NewMemToRegPass(PM);
   NewPrintCFGPass(PM);
   NewInstSelectPass(PM);
   NewMIRBuilderPass(PM);
   NewInstSchedPass(PM);
   NewDominatorTreeWrapperPass(PM);
}
