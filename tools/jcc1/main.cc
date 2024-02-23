#include <fstream>
#include <iostream>
#include <iterator>
#include <string>

#include "ast/AST.h"
#include "ast/AstNode.h"
#include "ast/DeclContext.h"
#include "grammar/Joos1WGrammar.h"
#include "jcc1.h"
#include "parsetree/ParseTreeVisitor.h"
#include "semantic/HierarchyChecker.h"
#include "semantic/NameResolver.h"
#include "utils/BumpAllocator.h"
#include "utils/CLI11.h"

diagnostics::DiagnosticEngine gDiag{};

void checkAndPrintErrors() {
   if(gDiag.hasErrors()) {
      for(auto m : gDiag.messages()) {
         m.emit(std::cerr);
         std::cerr << std::endl;
      }
      exit(1);
   }
}

int main(int argc, char** argv) {
   std::string optASTGraphFile, optPTGraphFile;
   bool optSilent = false;
   InputMode optInputMode = InputMode::Stdin;

   utils::CustomBufferResource CbResource{};
   BumpAllocator Alloc{&CbResource};
   ast::Semantic Sema{Alloc, gDiag};
   SourceManager SrcMgr{};

   CLI::App app{"Joos1W AST Printer"};
   app.add_option(
         "-a,--ast-dot", optASTGraphFile, "File to print the AST in DOT format");
   app.add_option("-p,--pt-dot",
                  optPTGraphFile,
                  "File to print the parse tree in DOT format");
   app.add_flag("-s,--silent",
                optSilent,
                "Only indicate if the input is valid or not and print any errors");
   app.add_flag("-x,--split",
                "Split the input file, contents delimited by ---, into multiple "
                "compilation units");
   app.allow_extras();
   CLI11_PARSE(app, argc, argv);

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
         SrcMgr.addFile(path);
      }
   } else {
      // Read std cin input until EOF
      std::string buffer_{std::istreambuf_iterator<char>(std::cin),
                          std::istreambuf_iterator<char>()};
      SrcMgr.emplaceBuffer();
      for(auto it = buffer_.begin(); it != buffer_.end(); ++it) {
         // If we encounter "---" in the input, split the input into multiple bufs
         if(*it == '-' && *(it + 1) == '-' && *(it + 2) == '-') {
            SrcMgr.emplaceBuffer(), it += 2;
         } else {
            SrcMgr.currentBuffer().push_back(*it);
         }
      }
   }

   // Work on each input buffer
   std::pmr::vector<ast::CompilationUnit*> compilation_units{Alloc};
   for(auto const& file : SrcMgr.files()) {
      // 1. Parse the input into a parse tree
      Joos1WParser parser{file, Alloc, &gDiag};
      parsetree::Node* pt = nullptr;
      int parseResult = parser.parse(pt);
      if(parseResult != 0 || pt == nullptr) {
         std::cerr << "Parsing failed with code: " << parseResult << " in file ";
         SrcMgr.print(std::cerr, file);
         std::cerr << std::endl;
         checkAndPrintErrors();
         return 42;
      }
      // 2. Run the AST visitor on the parse tree
      parsetree::ParseTreeVisitor visitor{Sema};
      ast::CompilationUnit* ast = nullptr;
      try {
         ast = visitor.visitCompilationUnit(pt);
      } catch(const parsetree::ParseTreeException& e) {
         std::cerr << "ParseTreeException: " << e.what() << " in file ";
         SrcMgr.print(std::cerr, file);
         std::cerr << std::endl;
         std::cerr << "Parse tree trace:" << std::endl;
         trace_node(e.get_where());
         return 1;
      }
      // 3. Check for errors
      checkAndPrintErrors();
      compilation_units.push_back(ast);
   }

   // Link the compilation units
   auto linking_unit = Sema.BuildLinkingUnit(compilation_units);
   if(!optASTGraphFile.empty()) {
      std::ofstream ast_file{optASTGraphFile};
      linking_unit->printDot(ast_file);
   }
   checkAndPrintErrors();

   // Run the name resolution pass
   semantic::NameResolver nameResolverPass{Alloc, gDiag, linking_unit};
   nameResolverPass.Resolve();
   checkAndPrintErrors();

   // Run the hierarchy checking pass
   semantic::HierarchyChecker hierarchyCheckerPass{gDiag, linking_unit};
   checkAndPrintErrors();

   // Print the AST to stdout at the end only
   if(optASTGraphFile.empty()) {
      linking_unit->print(std::cout);
   }

   return 0;
}
