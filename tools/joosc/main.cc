#include <unistd.h>

#include <iostream>
#include <string>

#include <ast/AST.h>
#include <ast/AstNode.h>
#include <ast/DeclContext.h>
#include <grammar/Joos1WGrammar.h>
#include <parsetree/ParseTreeVisitor.h>
#include <passes/AllPasses.h>
#include <semantic/HierarchyChecker.h>
#include <semantic/NameResolver.h>
#include <utils/BumpAllocator.h>
#include <third-party/CLI11.h>
#include <utils/PassManager.h>

int main(int argc, char** argv) {
   bool verbose = false;
   // Create the pass manager and source manager
   CLI::App app{"Joos1W Compiler for Marmoset", "joosc"};
   utils::PassManager PM{app};
   SourceManager SM{};
   app.add_flag("-c", "Dummy flag for compatibility with ./runtest.py");
   app.add_flag("-v", verbose, "Verbose");
   app.add_flag("--check-file-name", "Dummy flag that's always true")->default_val(true);
   app.allow_extras();
   CLI11_PARSE(app, argc, argv);

   if(verbose) {
      PM.Diag().setVerbose(100);
   }

   // Read the input into the source manager
   auto files{app.remaining()};
   for(auto const& path : files) {
      SM.addFile(path);
   }

   // Build the pass pipeline, ignoring command line options
   {
      using utils::Pass;
      Pass* p2 = nullptr;
      for(auto file : SM.files()) {
         auto* p1 = &NewJoos1WParserPass(PM, file, p2);
         p2 = &NewAstBuilderPass(PM, p1);
      }
      NewAstContextPass(PM);
      NewLinkerPass(PM);
      NewNameResolverPass(PM);
      NewHierarchyCheckerPass(PM);
      // Explicitly enable the pass that we want to run
      PM.PO().EnablePass("sema-hier");
   }

   // Run the passes
   if(!PM.Run()) {
      std::cerr << "Error running pass: " << PM.LastRun()->Desc() << std::endl;
      if(PM.Diag().hasErrors()) {
         for(auto m : PM.Diag().errors()) {
            m.emit(std::cerr);
            std::cerr << std::endl;
         }
      }
      return 42;
   }
   
   return 0;
}
