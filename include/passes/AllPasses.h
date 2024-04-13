#pragma once

#include <diagnostics/SourceManager.h>
#include <utils/PassManager.h>
#include <tir/CompilationUnit.h>

/* ===--------------------------------------------------------------------=== */
// Front-end passes
/* ===--------------------------------------------------------------------=== */

utils::Pass& NewJoos1WParserPass(utils::PassManager& PM, SourceFile file,
                                 utils::Pass* depends);
utils::Pass& NewAstBuilderPass(utils::PassManager& PM, utils::Pass* depends);

DECLARE_PASS(HierarchyCheckerPass);
DECLARE_PASS(AstContextPass);
DECLARE_PASS(LinkerPass);
DECLARE_PASS(NameResolverPass);
DECLARE_PASS(PrintASTPass);
DECLARE_PASS(ExprResolverPass);
DECLARE_PASS(DFAPass);

/* ===--------------------------------------------------------------------=== */
// Optimization passes
/* ===--------------------------------------------------------------------=== */

utils::Pass& NewIRContextPass(utils::PassManager&, tir::CompilationUnit&);

DECLARE_PASS(SimplifyCFG);
DECLARE_PASS(GlobalDCE);
DECLARE_PASS(MemToReg);
DECLARE_PASS(PrintCFG);
DECLARE_PASS(AsmWriter);

/* ===--------------------------------------------------------------------=== */
// Functions to automatically add all these passes
/* ===--------------------------------------------------------------------=== */

/// @brief Adds all the front-end passes EXCEPT for parsing passes (per file)
static void BuildFrontEndPasses(utils::PassManager&PM) {
   NewAstContextPass(PM);
   NewLinkerPass(PM);
   NewPrintASTPass(PM);
   NewNameResolverPass(PM);
   NewHierarchyCheckerPass(PM);
   NewExprResolverPass(PM);
   NewDFAPass(PM);
}

/// @brief Adds all the optimization passes EXCEPT for the context pass
static void BuildOptPasses(utils::PassManager& PM) {
   NewSimplifyCFG(PM);
   NewGlobalDCE(PM);
   NewMemToReg(PM);
   NewPrintCFG(PM);
   NewAsmWriter(PM);
}
