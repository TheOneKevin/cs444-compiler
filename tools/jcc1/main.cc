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

#include <iostream>
#include <iterator>
#include <string>

enum class InputMode { File, Stdin };

int main(int argc, char** argv) {
   InputMode optInputMode = InputMode::Stdin;

   // Create the pass manager and source manager
   CLI::App app{"Joos1W Compiler Frontend", "jcc1"};
   utils::PassManager PM{app};
   SourceManager SM{};

   // clang-format off
   // 1. Build the pass-specific global command line options
   app.add_flag("--print-dot", "If a printing pass is run, print any trees in DOT format");
   app.add_option("--print-output", "If a printing pass is run, will write the output to this file or directory");
   app.add_flag("--print-split", "If a printing pass is run, split the output into multiple files");
   // 2. Build the jcc1-specific command line options
   app.add_flag("-x,--split", "Split the input file whose contents are delimited\nby \"---\" into multiple compilation units");
   auto optVerboseLevel = app.add_flag("-v", "Set the verbosity level")
      ->expected(0, 1)
      ->check(CLI::Range(0, 3));
   app.allow_extras();
   // clang-format on

   // Build pass pipeline that requires command line options
   {
      NewAstContextPass(PM);
      NewLinkerPass(PM);
      NewPrintASTPass(PM);
      NewNameResolverPass(PM);
      NewHierarchyCheckerPass(PM);
      PM.PO().AddAllOptions();
      PM.PO().EnablePass("sema-hier");
   }

   // Parse the command line options
   CLI11_PARSE(app, argc, argv);

   // Validate the command line options
   {
      auto split = app.count("--print-split");
      auto output = app.get_option("--print-output")->reduced_results();
      // If print-split is set, the print-output must be set
      if(split && output.empty()) {
         std::cerr << "Error: --print-split requires --print-output" << std::endl;
         return 1;
      }
      // If print-split is set, the print-dot must be set
      if(split && !app.count("--print-dot")) {
         std::cerr << "Error: --print-split requires --print-dot" << std::endl;
         return 1;
      }
   }

   // Set the verbosity of the diagnostic engine
   for(auto r : optVerboseLevel->results()) {
      int level = 0;
      if(r == "true") {
         level = 1;
      } else {
         try {
            level = std::stoi(r);
         } catch(std::invalid_argument const& e) {
            std::cerr << "Invalid verbosity level: " << r << std::endl;
            return 1;
         }
      }
      PM.Diag().setVerbose(level);
   }

   // Ensure the remaining arguments are all valid paths
   auto files{app.remaining()};
   for(auto const& path : files) {
      if(access(path.c_str(), F_OK) == -1) {
         std::cerr << "File " << path << " does not exist" << std::endl;
         return 1;
      }
      optInputMode = InputMode::File;
   }

   // Read the input into the source manager (either from file or stdin)
   if(optInputMode == InputMode::File) {
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

   // Build the remaining pass pipeline that will input the source manager
   {
      using utils::Pass;
      Pass* p2 = nullptr;
      for(auto file : SM.files()) {
         auto* p1 = &NewJoos1WParserPass(PM, file, p2);
         p2 = &NewAstBuilderPass(PM, p1);
      }
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
