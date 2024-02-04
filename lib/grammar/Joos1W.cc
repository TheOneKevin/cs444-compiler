#include "grammar/Joos1WGrammar.h"
#include "parsetree/ParseTree.h"

using Node = parsetree::Node;
using Operator = parsetree::Operator;
using Literal = parsetree::Literal;
using Identifier = parsetree::Identifier;
using Modifier = parsetree::Modifier;
using BasicType = parsetree::BasicType;

Node* Joos1WLexer::make_poison() {
   auto ret = new Node{Node::Type::Poison};
   nodes.push_back(ret);
   return ret;
}

Node* Joos1WLexer::make_operator(Operator::Type type) {
   auto ret = new Operator{type};
   nodes.push_back(ret);
   return ret;
}

Node* Joos1WLexer::make_literal(Literal::Type type, char const* value) {
   auto ret = new Literal{type, value};
   nodes.push_back(ret);
   return ret;
}

Node* Joos1WLexer::make_identifier(char const* name) {
   auto ret = new Identifier{name};
   nodes.push_back(ret);
   return ret;
}

Node* Joos1WLexer::make_modifier(Modifier::Type type) {
   auto ret = new Modifier{type};
   nodes.push_back(ret);
   return ret;
}

Node* Joos1WLexer::make_basic_type(BasicType::Type type) {
   auto ret = new BasicType{type};
   nodes.push_back(ret);
   return ret;
}
