#include <gtest/gtest.h>
#include "common.h"
#include "utils/BumpAllocator.h"

TEST(AstTests, CanonicalName) {
   auto node = testing::get_ast(R"(
      package org.example.joos1w;
      public class MyClass {
         public int myField;
         static public String anotherStaticField;
         public MyClass() {}
         public void myMethod() {
            int myLocal;
         }
         public static void anotherStaticMethod() {
            int anotherLocal;
         }
      }
   )");
   ASSERT_NE(node, nullptr);
   auto body = dynamic_cast<ast::ClassDecl*>(node->body());
   ASSERT_NE(body, nullptr);
   ASSERT_TRUE(body->hasCanonicalName());
   ASSERT_EQ(body->getCanonicalName(), "org.example.joos1w.MyClass");
}
