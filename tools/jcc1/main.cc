#include <ast/AST.h>
#include <ast/AstNode.h>
#include <ast/DeclContext.h>
#include <grammar/Joos1WGrammar.h>
#include <parsetree/ParseTreeVisitor.h>
#include <passes/AllPasses.h>
#include <semantic/HierarchyChecker.h>
#include <semantic/NameResolver.h>
#include <third-party/CLI11.h>
#include <utils/BumpAllocator.h>
#include <utils/PassManager.h>

#include <filesystem>
#include <iostream>
#include <iterator>
#include <string>

#include "diagnostics/Diagnostics.h"

enum class InputMode { File, Stdin };
void pretty_print_errors(SourceManager& SM, diagnostics::DiagnosticEngine& diag);

int main(int argc, char** argv) {
   InputMode optInputMode = InputMode::Stdin;
   std::string optStdlibPath = "/u/cs444/pub/stdlib/6.1/";
   bool optSplit = false;
   bool optCompile = false;
   bool optFreestanding = false;
   bool optHeapReuse = true;

   // Create the pass manager and source manager
   CLI::App app{"Joos1W Compiler Frontend", "jcc1"};
   utils::PassManager PM{app};
   SourceManager SM{};

   // clang-format off
   // 1. Build the pass-specific global command line options
   app.add_flag("--print-dot", "If a printing pass is run, print any trees in DOT format");
   app.add_option("--print-output", "If a printing pass is run, will write the output to this file or directory");
   app.add_flag("--print-split", "If a printing pass is run, split the output into multiple files");
   app.add_flag("--print-ignore-std", "If a printing pass is run, ignore the standard library");
   app.add_flag("--check-file-name", "Check if the file name matches the class name");
   // 2. Build the jcc1-specific command line options
   app.add_flag("-x,--split", optSplit, "Split the input file whose contents are delimited\nby \"---\" into multiple compilation units");
   auto optVerboseLevel = app.add_flag("-v", "Set the verbosity level")
      ->expected(0, 1)
      ->check(CLI::Range(0, 3));
   app.add_flag("-c", optCompile, "Add the passes to compile the input file(s) to an executable");
   app.add_option("--stdlib", optStdlibPath, "The path to the standard library to use for compilation")
      ->check(CLI::ExistingDirectory)
      ->expected(0, 1)
      ->capture_default_str();
   app.add_flag("--freestanding", optFreestanding, "Compile the input file(s) to a freestanding executable, without the standard library");
   app.add_flag("--no-heap-reuse", optHeapReuse, "Do not reuse heap memory between passes (for debugging heap GC issues)");
   app.allow_extras();
   // clang-format on

   // Build pass pipeline that requires command line options
   {
      NewAstContextPass(PM);
      NewLinkerPass(PM);
      NewPrintASTPass(PM);
      NewNameResolverPass(PM);
      NewHierarchyCheckerPass(PM);
      NewExprResolverPass(PM);
      NewDFAPass(PM);
      PM.PO().AddAllOptions();
   }

   // Parse the command line options
   CLI11_PARSE(app, argc, argv);

   // Disable heap reuse if requested
   if(optHeapReuse) PM.setHeapReuse(false);

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
      // If we choose to split the input or not
      if(optSplit) {
         for(auto it = buffer_.begin(); it != buffer_.end(); ++it) {
            // If we see "---" in the input, split the input into multiple bufs
            if(*it == '-' && *(it + 1) == '-' && *(it + 2) == '-') {
               SM.emplaceBuffer(), it += 2;
            } else {
               SM.currentBuffer().push_back(*it);
            }
         }
      } else {
         SM.currentBuffer().insert(
               SM.currentBuffer().end(), buffer_.begin(), buffer_.end());
      }
   }

   // Add the standard library to the source manager by recursively searching
   // for .java files in the stdlib path
   if(!optFreestanding) {
      namespace fs = std::filesystem;
      fs::recursive_directory_iterator it{optStdlibPath};
      fs::recursive_directory_iterator end;
      for(; it != end; ++it) {
         if(it->is_regular_file() && it->path().extension() == ".java") {
            SM.addFile(it->path().string());
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

   // If we are compiling, enable the appropriate passes
   if(optCompile) {
      PM.PO().EnablePass("sema-expr");
   }

   // Run the passes
   if(!PM.Run()) {
      std::cerr << "Error running pass: " << PM.LastRun()->Desc() << std::endl;
      if(PM.Diag().hasErrors()) {
         pretty_print_errors(SM, PM.Diag());
      }
      return 42;
   }
   return 0;
}
