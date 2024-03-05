#include <filesystem>
#include <string_view>

#include "CompilerPasses.h"
#include "diagnostics/Diagnostics.h"
#include "diagnostics/Location.h"
#include "diagnostics/SourceManager.h"
#include "semantic/NameResolver.h"
#include "semantic/Semantic.h"
#include "third-party/CLI11.h"
#include "utils/BumpAllocator.h"
#include "utils/PassManager.h"

using std::string_view;
using utils::Pass;
using utils::PassManager;
using namespace joos1;

class PrintASTPass final : public Pass {
public:
   PrintASTPass(PassManager& PM) noexcept : Pass(PM) {
      optDot = PM.PO().GetExistingOption("--print-dot");
      optOutput = PM.PO().GetExistingOption("--print-output");
      optSplit = PM.PO().GetExistingOption("--print-split");
      optIgnoreStd = PM.PO().GetExistingOption("--print-ignore-std");
   }
   string_view Name() const override { return "print-ast"; }
   string_view Desc() const override { return "Print AST"; }

   void Run() override {
      // 1a. Grab the AST node
      auto& pass = GetPass<LinkerPass>();
      auto* node = pass.LinkingUnit();
      if(!node) return;
      // 1b. Create a new heap to build a new AST if we're ignoring stdlib
      BumpAllocator alloc{NewHeap()};
      ast::Semantic newSema{alloc, PM().Diag()};
      if(optIgnoreStd && optIgnoreStd->count()) {
         std::pmr::vector<ast::CompilationUnit*> cus{alloc};
         for(auto* cu : node->compliationUnits())
            if(!cu->isStdLib()) cus.push_back(cu);
         node = newSema.BuildLinkingUnit(cus);
      }
      // 2. Interpret the command line options
      bool printDot = optDot && optDot->count();
      bool printSplit = optSplit && optSplit->count();
      std::string outputPath = optOutput ? optOutput->as<std::string>() : "";
      // 3. Print the AST depending on the options
      if(!printSplit) {
         // 1. Now we try to open the output file if exists
         std::ofstream file{};
         if(!outputPath.empty()) {
            file = std::ofstream{outputPath};
            if(!file.is_open()) {
               PM().Diag().ReportError(SourceRange{})
                     << "failed to open output file " << outputPath;
               return;
            }
         }
         // 2. Set the output stream
         std::ostream& os = file.is_open() ? file : std::cout;
         // 3. Now we can print the AST
         if(printDot)
            node->printDot(os);
         else
            node->print(os);
      } else {
         namespace fs = std::filesystem;
         // 1. Create the output directory if it doesn't exist
         fs::create_directories(outputPath);
         // 2. Create a new file for each compilation unit
         for(auto const& cu : node->compliationUnits()) {
            if(!cu->body() || !cu->bodyAsDecl()->hasCanonicalName()) continue;
            std::string canonName{cu->bodyAsDecl()->getCanonicalName()};
            auto filepath = fs::path{outputPath} / fs::path{canonName + ".dot"};
            std::ofstream file{filepath};
            if(!file.is_open()) {
               PM().Diag().ReportError(SourceRange{})
                     << "failed to open output file " << filepath.string();
               return;
            }
            // 3. Set the output stream
            std::ostream& os = file;
            // 4. Now we can print the AST
            cu->printDot(os);
         }
      }
   }

private:
   void computeDependencies() override {
      ComputeDependency(GetPass<LinkerPass>());
      ComputeDependency(GetPass<AstContextPass>());
   }
   CLI::Option *optDot, *optOutput, *optSplit, *optIgnoreStd;
};

REGISTER_PASS(PrintASTPass);
