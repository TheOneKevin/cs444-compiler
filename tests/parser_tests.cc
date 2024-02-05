#include <gtest/gtest.h>

#include <iostream>

#include "common.h"
#include "grammar/Joos1WGrammar.h"
#include "parsetree/ParseTree.h"
#include "tools/genfrags/basic_fragments.h"
#include "tools/genfrags/class_fragments.h"
#include "tools/genfrags/method_fragments.h"
#include "utils/BumpAllocator.h"

namespace {

// FIXME(kevin): This is a hack! We should move this to custom.cc instead
bool my_parse_grammar(const string& str, parsetree::BumpAllocator& alloc) {
   Joos1WParser parser{str, alloc};
   parsetree::Node* parse_tree = nullptr;
   int result = parser.parse(parse_tree);
   if(!parse_tree || result != 0) return false;
   return true;
}

}; // namespace

TEST(ParserTests, SimpleGrammar) {
   testing::BasicGrammarGenerator g{};
   CustomBufferResource mbr{4096};
   parsetree::BumpAllocator alloc{&mbr};
   for(auto x : g.match_string("class T{void f(){$<stmt>$}}")) {
      mbr.reset();
      if(!::my_parse_grammar(x, alloc)) {
         std::cout << "Failed to parse: " << x << std::endl;
         EXPECT_TRUE(false);
         return;
      }
      EXPECT_TRUE(true);
   }
}

TEST(ParserTests, ClassModifiers) {
   testing::ClassGrammarGenerator g{};
   for(auto x : g.match_string("$<class>$")) {
      if(!testing::parse_grammar(x)) {
         std::cout << "Failed to parse: " << x << std::endl;
         EXPECT_TRUE(false);
      }
      EXPECT_TRUE(true);
   }
}

TEST(ParserTests, InterfaceModifiers) {
   testing::ClassGrammarGenerator g{};
   for(auto x : g.match_string("$<intf>$")) {
      if(!testing::parse_grammar(x)) {
         std::cout << "Failed to parse: " << x << std::endl;
         EXPECT_TRUE(false);
      }
      EXPECT_TRUE(true);
   }
}

TEST(ParserTests, ClassMethods) {
   testing::MethodGrammarGenerator g{};
   for(auto x : g.match_string("class T{$<class_method>$}")) {
      if(!testing::parse_grammar(x)) {
         std::cout << "Failed to parse: " << x << std::endl;
         EXPECT_TRUE(false);
      }
      EXPECT_TRUE(true);
   }
}

TEST(ParserTests, InterfaceMethods) {
   testing::MethodGrammarGenerator g{};
   for(auto x : g.match_string("interface T{$<interface_method>$}")) {
      if(!testing::parse_grammar(x)) {
         std::cout << "Failed to parse: " << x << std::endl;
         EXPECT_TRUE(false);
      }
      EXPECT_TRUE(true);
   }
}
