#include "common.h"

#include <gtest/gtest.h>

#include "grammar/Joos1WGrammar.h"
#include "parsetree/ParseTreeVisitor.h"

bool testing::parse_grammar(const string& str) {
   Joos1WParser parser{str};
   parsetree::Node* parse_tree = nullptr;
   int result = parser.parse(parse_tree);
   if(!parse_tree || result != 0) return false;
   return true;
}

bool testing::build_ast(const string& str) {
   Joos1WParser parser{str};
   parsetree::Node* parse_tree = nullptr;
   int result = parser.parse(parse_tree);
   if(!parse_tree || result != 0) return false;
   try {
      // FIXME(kevin): We should fix the allocator API... this is quite ugly
      std::pmr::monotonic_buffer_resource mbr{};
      BumpAllocator alloc{&mbr};
      parsetree::ParseTreeVisitor visitor{alloc};
      auto ast = visitor.visitCompilationUnit(parse_tree);
      (void)ast;
      return true;
   } catch(...) {
      return false;
   }
}
