#include <ast/AST.h>
#include <ast/AstNode.h>
#include <ast/DeclContext.h>
#include <grammar/Joos1WGrammar.h>
#include <parsetree/ParseTreeVisitor.h>
#include <passes/AllPasses.h>
#include <semantic/HierarchyChecker.h>
#include <semantic/NameResolver.h>
#include <utils/BumpAllocator.h>
#include <utils/CLI11.h>
#include <utils/PassManager.h>

#include <fstream>
#include <iostream>
#include <iterator>
#include <string>

#include "jcc1.h"

void checkAndPrintErrors(diagnostics::DiagnosticEngine& d) {
   if(d.hasErrors()) {
      for(auto m : d.errors()) {
         m.emit(std::cerr);
         std::cerr << std::endl;
      }
      exit(1);
   }
}

int main(int argc, char** argv) {
   // Command line option variables
   std::string optASTGraphFile, optPTGraphFile;
   bool optASTDump = false;
   jcc1::InputMode optInputMode = jcc1::InputMode::Stdin;
   bool optVerbose = false;

   // Parse command line options
   CLI::App app{"Joos1W AST Printer"};
   app.add_option(
         "--dot-ast", optASTGraphFile, "File to print the AST in DOT format");
   app.add_option(
         "--dot-pt", optPTGraphFile, "File to print the parse tree in DOT format");
   app.add_flag("--dump-ast", optASTDump, "Dump the AST to standard output");
   app.add_flag("-x,--split",
                "Split the input file, contents delimited by ---, into multiple "
                "compilation units");
   app.add_flag("-v,--verbose", optVerbose, "Enable verbose output");
   app.allow_extras();
   CLI11_PARSE(app, argc, argv);

   // Persistent parser objects
   utils::PassManager PM{};
   SourceManager SM{};

   // Set the verbosity of the diagnostic engine
   PM.Diag().setVerbose(optVerbose);

   // Ensure the remaining arguments are all valid paths
   auto files{app.remaining()};
   for(auto const& path : files) {
      if(access(path.c_str(), F_OK) == -1) {
         std::cerr << "File " << path << " does not exist" << std::endl;
         return 1;
      }
      optInputMode = jcc1::InputMode::File;
   }

   // Read the input into the source manager (either from file or stdin)
   if(optInputMode == jcc1::InputMode::File) {
      for(auto const& path : files) {
         SM.addFile(path);
      }
   } else {
      // Read std cin input until EOF
      std::string buffer_{std::istreambuf_iterator<char>(std::cin),
                          std::istreambuf_iterator<char>()};
      SM.emplaceBuffer();
      for(auto it = buffer_.begin(); it != buffer_.end(); ++it) {
         // If we encounter "---" in the input, split the input into multiple bufs
         if(*it == '-' && *(it + 1) == '-' && *(it + 2) == '-') {
            SM.emplaceBuffer(), it += 2;
         } else {
            SM.currentBuffer().push_back(*it);
         }
      }
   }

   // Open the output file if requested
   std::ofstream astGraphFile;
   if(!optASTGraphFile.empty()) {
      astGraphFile.open(optASTGraphFile);
   }

   // Build the passes
   {
      using utils::Pass;
      Pass* p2 = nullptr;
      for(auto file : SM.files()) {
         auto* p1 = &NewJoos1WParserPass(PM, file, p2);
         p2 = &NewAstBuilderPass(PM, p1);
      }
      NewAstContextPass(PM);
      auto* p3 = &NewLinkerPass(PM);
      if(astGraphFile.is_open()) {
         NewPrintASTPass(PM, p3, astGraphFile, true);
      } else if(optASTDump) {
         NewPrintASTPass(PM, p3, std::cout, false);
      }
      NewNameResolverPass(PM);
      NewHierarchyCheckerPass(PM);
   }

   // Run the passes
   if(!PM.Run()) {
      std::cerr << "Error running pass: " << PM.LastRun()->Name() << std::endl;
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
