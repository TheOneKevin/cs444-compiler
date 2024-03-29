#pragma once

#include <diagnostics/SourceManager.h>
#include <utils/PassManager.h>
#include "tir/Constant.h"

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

// Optimization passes

DECLARE_PASS(SimplifyCFG);
DECLARE_PASS(GlobalDCE);
utils::Pass& NewIRContextPass(utils::PassManager&, tir::CompilationUnit&);
