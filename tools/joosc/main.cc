#include <unistd.h>

#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>

#include "ast/AST.h"
#include "grammar/Joos1WGrammar.h"
#include "parsetree/ParseTree.h"
#include "parsetree/ParseTreeVisitor.h"
#include "semantic/NameResolver.h"
#include "semantic/HierarchyChecker.h"

// FIXME(kevin): Remove when we have proper AST handling
bool isLiteralTypeValid(parsetree::Node* node) {
   if(node == nullptr) return true;
   if(node->get_node_type() == parsetree::Node::Type::Literal)
      return static_cast<parsetree::Literal*>(node)->isValid();
   for(size_t i = 0; i < node->num_children(); i++) {
      if(!isLiteralTypeValid(node->child(i))) return false;
   }
   return true;
}

int main(int argc, char** argv) {
   bool printDot = false;

   // Check for correct number of arguments
   if(argc < 2) {
      std::cerr << "Usage: " << argv[0] << " input-file " << std::endl;
      exit(EXIT_FAILURE);
   }

   std::pmr::monotonic_buffer_resource mbr{};
   BumpAllocator alloc{&mbr};
   diagnostics::DiagnosticEngine diag{};
   ast::Semantic sem{alloc, diag};
   parsetree::ParseTreeVisitor visitor{sem};

   std::pmr::vector<ast::CompilationUnit*> asts;

   for (int i = 1; i < argc; i++) {
      if (std::string(argv[i]) == "-x") {
         printDot = true;
         continue;
      }
      // Check if the file is a .java file
      std::string filePath = argv[i];
      std::string fileName = filePath.substr(filePath.find_last_of("/\\") + 1);
      if(filePath.substr(filePath.length() - 5, 5) != ".java") {
         std::cerr << "Error: not a valid .java file" << std::endl;
         return 42;
      }

      std::string str;

      // Read file from command-line argument
      try {
         // Read entire input
         std::ifstream inputFile(filePath);
         std::stringstream buffer;
         buffer << inputFile.rdbuf();
         str = buffer.str();
      } catch(...) {
         std::cerr << "Error! Could not open input file \"" << filePath << "\""
                  << std::endl;
         exit(EXIT_FAILURE);
      }

      // Check for non-ASCII characters
      for(unsigned i = 0; i < str.length(); i++) {
         if(static_cast<unsigned char>(str[i]) > 127) {
            std::cerr << "Parse error: non-ASCII character in input" << std::endl;
            return 42;
         }
      }

      // Parse the input
      parsetree::Node* parse_tree = nullptr;
      Joos1WParser parser{str};
      int result = parser.parse(parse_tree);

      // Check to see if the parse is success
      if(!parse_tree || result) {
         return 42;
      }

      // Check if the parse tree is poisoned
      if(parse_tree->is_poisoned()) {
         std::cerr << "Parse error: parse tree is poisoned" << std::endl;
         return 42;
      }

      // Check if the parse tree has valid literal types
      if(!isLiteralTypeValid(parse_tree)) {
         std::cerr << "Parse error: invalid literal type" << std::endl;
         return 42;
      }

      if(diag.hasErrors()) {
         for(auto& msg : diag.messages()) {
            msg.emit(std::cerr) << std::endl;
         }
         return 42;
      }
      
      // Build the AST from the parse tree
      ast::CompilationUnit* ast = nullptr;
      try {
         ast = visitor.visitCompilationUnit(parse_tree);
      } catch(const std::runtime_error& re) {
         std::cerr << "Runtime error: " << re.what() << std::endl;
         return 42;
      } catch(...) {
         std::cerr << "Unknown failure occurred." << std::endl;
         return 1;
      }
      if(!ast) return 42;

      // Check the AST to make sure the class/intf has the same name as file
      auto cuBody = dynamic_cast<ast::Decl*>(ast->body());
      fileName = fileName.substr(0, fileName.length() - 5);
      if(cuBody->name() != fileName) {
         std::cerr << "Parse error: class/interface name does not match file name"
                  << std::endl;
         std::cerr << "Class/interface name: " << cuBody->name() << std::endl;
         std::cerr << "File name: " << fileName << std::endl;
         return 42;
      }

      if(diag.hasErrors()) {
         for(auto& msg : diag.messages()) {
            msg.emit(std::cerr) << std::endl;
         }
         return 42;
      }

      asts.push_back(ast);
   }

   

   ast::LinkingUnit* linkingUnit = sem.BuildLinkingUnit(asts);
   
   if(diag.hasErrors()) {
      for(auto& msg : diag.messages()) {
         msg.emit(std::cerr) << std::endl;
      }
      return 42;
   }
   
   if (printDot) {
      linkingUnit->printDot(std::cout);
   }

   semantic::NameResolver resolver{alloc, diag, linkingUnit};

   resolver.Resolve();

   if(diag.hasErrors()) {
      for(auto& msg : diag.messages()) {
         msg.emit(std::cerr) << std::endl;
      }
      return 42;
   }

   semantic::HierarchyChecker hierarchy{diag, linkingUnit};

   if(diag.hasErrors()) {
      for(auto& msg : diag.messages()) {
         msg.emit(std::cerr) << std::endl;
      }
      return 42;
   }

   return 0;
}
