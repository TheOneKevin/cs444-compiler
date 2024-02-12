#include <unistd.h>

#include <iostream>
#include <iterator>
#include <string>

#include "ast/AST.h"
#include "grammar/Joos1WGrammar.h"
#include "parsetree/ParseTreeVisitor.h"
#include "utils/CommandLine.h"

int main() {
   // Read entire input until enter
   std::istreambuf_iterator<char> eos;
   std::string str = std::string(std::istreambuf_iterator<char>(std::cin), eos);

   // Parse input
   Joos1WParser parser{str};
   parsetree::Node* parse_tree = nullptr;
   int result = parser.parse(parse_tree);
   std::cout << "Result: " << result << std::endl;

   if(result != 0 || parse_tree == nullptr) {
      std::cout << "Parsing failed" << std::endl;
      return 1;
   }

   // FIXME(kevin): We should fix the allocator API... this is quite ugly
   std::pmr::monotonic_buffer_resource mbr{};
   BumpAllocator alloc{&mbr};
   parsetree::ParseTreeVisitor visitor{alloc};
   ast::CompilationUnit* ast = visitor.visitCompilationUnit(parse_tree);
   ast->print(std::cout);

   return 0;
}
