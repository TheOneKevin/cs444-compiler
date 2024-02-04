#include <gtest/gtest.h>

#include <string>

#include "common.h"

constexpr auto invalid_bodies = {
      // Invalid instanceof usage
      "return x instanceof int;",
      "return x instanceof void;",
      "return x instanceof null;",

      // Integer too big
      "int x = 2147483648;",
      "int x = -2147483648;",
      "int x = 9999999999999999 + 99;",
      "int x = -9999999999999999 - 99;",

      // Floats and doubles are not allowed
      "float x = 3.14;",
      "double x = -3.14;",
      "int x = 3.14;",

      // Multiple fields are not allowed
      "int x, y, z;",
};

constexpr auto valid_bodies = {
      // Valid instanceof usage
      "return x instanceof int[];",
      "return x instanceof Obj;",
      "return x instanceof Obj[];",

      // Integer OK size
      "int x = 2147483647;",
      "int x = -2147483647;",

      // Assignment is allowed
      "int x = y = z = 10;"};

TEST(ParserTests, MoreInvalidExpressions) {
   for(auto body : invalid_bodies) {
      std::string input =
            "class Test { void test() { " + std::string{body} + " } }";
      auto result = testing::parse_grammar(input);
      EXPECT_FALSE(result);
   }
}

TEST(ParserTests, MoreValidExpressions) {
   for(auto body : valid_bodies) {
      std::string input =
            "class Test { void test() { " + std::string{body} + " } }";
      auto result = testing::parse_grammar(input);
      EXPECT_TRUE(result);
   }
}
