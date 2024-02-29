#pragma once

#include <diagnostics/SourceManager.h>
#include <utils/PassManager.h>

utils::Pass& NewJoos1WParserPass(utils::PassManager& PM, SourceFile file,
                                 utils::Pass* depends);
utils::Pass& NewAstContextPass(utils::PassManager& PM);
utils::Pass& NewAstBuilderPass(utils::PassManager& PM, utils::Pass* depends);
utils::Pass& NewLinkerPass(utils::PassManager& PM);
utils::Pass& NewNameResolverPass(utils::PassManager& PM);
utils::Pass& NewHierarchyCheckerPass(utils::PassManager& PM);
utils::Pass& NewPrintASTPass(utils::PassManager& PM);
utils::Pass& NewExprResolverPass(utils::PassManager& PM);
