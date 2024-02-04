#include <gtest/gtest.h>

#include <iostream>

#include "common.h"
#include "tools/genfrags/basic_fragments.h"
#include "tools/genfrags/class_fragments.h"
#include "tools/genfrags/method_fragments.h"

TEST(ParserTests, SimpleGrammar) {
   testing::BasicGrammarGenerator g{};
   for(auto x : g.match_string("class T{void f(){$<stmt>$}}")) {
      if(!testing::parse_grammar(x)) {
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
