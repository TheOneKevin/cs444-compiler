#pragma once

#include <memory_resource>
#include <vector>

#include "joos1w.parser.tab.h"
#include "parsetree/ParseTree.h"

class Joos1WParser;

class Joos1WLexer : public yyFlexLexer {
   using Node = parsetree::Node;
   using Operator = parsetree::Operator;
   using Literal = parsetree::Literal;
   using Identifier = parsetree::Identifier;
   using Modifier = parsetree::Modifier;
   using BasicType = parsetree::BasicType;
   friend class Joos1WParser;

private:
   Joos1WLexer(parsetree::BumpAllocator& alloc) : yycolumn{1}, alloc{alloc} {}

public:
   // This is the generate Flex lexer function
   int yylex();
   // This is a bison-specific lexer function, implemented in the .l file
   int bison_lex(YYSTYPE* lvalp, YYLTYPE* llocp);

   /// @brief Wrapper around the node constructor to keep track of all nodes
   /// @param ...args The arguments to the node constructor
   /// @return The newly created node
   template <typename... Args>
   Node* make_node(Node::Type type, Args&&... args) {
      void* bytes = alloc.allocate_bytes(sizeof(Node));
      return new(bytes) Node(alloc, type, std::forward<Args>(args)...);
   }
   Node* make_leaf(Node::Type type) {
      void* bytes = alloc.allocate_bytes(sizeof(Node));
      return new(bytes) Node(type);
   }
   /// @brief See make_node
   Node* make_poison();
   /// @brief See make_node
   Node* make_operator(Operator::Type type);
   /// @brief See make_node
   Node* make_literal(Literal::Type type, char const* value);
   /// @brief See make_node
   Node* make_identifier(char const* name);
   /// @brief See make_node
   Node* make_modifier(Modifier::Type type);
   /// @brief See make_node
   Node* make_basic_type(BasicType::Type type);

private:
   // This is a private function that is called by the lexer to handle comments
   // It is implemented in the .l file
   void comment();

private:
   YYLTYPE yylloc;
   YYSTYPE yylval;
   int yycolumn;
   parsetree::BumpAllocator& alloc;
};
