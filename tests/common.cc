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
      auto ast = parsetree::visitCompilationUnit(parse_tree);
      (void)ast;
      return true;
   } catch(...) {
      return false;
   }
}
