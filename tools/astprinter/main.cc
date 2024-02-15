#include <unistd.h>

#include <iostream>
#include <iterator>
#include <string>

#include "ast/AST.h"
#include "grammar/Joos1WGrammar.h"
#include "parsetree/ParseTreeVisitor.h"
#include "utils/CommandLine.h"

static inline void trace_node(parsetree::Node const* node) {
   if(node->parent() != nullptr) {
      trace_node(node->parent());
      std::cerr << " -> ";
   }
   std::cerr << node->type_string() << std::endl;
}

static inline void mark_node(parsetree::Node* node) {
   if(!node)
      return;
   mark_node(node->parent());
   node->mark();
}

int main(int argc, char** argv) {
   utils::InputParser input(argc, argv);

   // Read entire input until enter
   std::istreambuf_iterator<char> eos;
   std::string str = std::string(std::istreambuf_iterator<char>(std::cin), eos);

   // Flag to print the parse tree in dot format
   bool print_dot = false;
   if(input.cmdOptionExists("-x")) {
      print_dot = true;
   }

   // Parse input
   Joos1WParser parser{str};
   parsetree::Node* parse_tree = nullptr;
   int result = parser.parse(parse_tree);
   if(!print_dot) {
      std::cout << "Result: " << result << std::endl;
   }

   if(result != 0 || parse_tree == nullptr) {
      std::cout << "Parsing failed" << std::endl;
      return 1;
   }

   // FIXME(kevin): We should fix the allocator API... this is quite ugly
   std::pmr::monotonic_buffer_resource mbr{};
   BumpAllocator alloc{&mbr};
   diagnostics::DiagnosticEngine diag{};
   ast::Semantic sem{alloc, diag};
   parsetree::ParseTreeVisitor visitor{sem};
   ast::CompilationUnit* ast = nullptr;
   try {
      ast = visitor.visitCompilationUnit(parse_tree);
   } catch(const parsetree::ParseTreeException& e) {
      std::cerr << "ParseTreeException: " << e.what() << std::endl;
      if(print_dot) {
         mark_node(e.get_where());
         parse_tree->printDot(std::cout);
      } else {
         trace_node(e.get_where());
      }
      return 1;
   }

   if(diag.hasErrors()) {
      for(auto m : diag.messages()) {
         m.emit(std::cerr);
         std::cerr << std::endl;
      }
      return 1;
   }

   if(print_dot) {
      ast->printDot(std::cout);
   } else {
      ast->print(std::cout);
   }

   return 0;
}
