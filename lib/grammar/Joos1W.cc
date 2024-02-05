#include "grammar/Joos1WGrammar.h"
#include "parsetree/ParseTree.h"

using Node = parsetree::Node;
using Operator = parsetree::Operator;
using Literal = parsetree::Literal;
using Identifier = parsetree::Identifier;
using Modifier = parsetree::Modifier;
using BasicType = parsetree::BasicType;

Node* Joos1WLexer::make_poison() {
   void* bytes = alloc.allocate_bytes(sizeof(Node));
   return new(bytes) Node(Node::Type::Poison);
}

Node* Joos1WLexer::make_operator(Operator::Type type) {
   void* bytes = alloc.allocate_bytes(sizeof(Operator));
   return new(bytes) Operator(type);
}

Node* Joos1WLexer::make_literal(Literal::Type type, char const* value) {
   void* bytes = alloc.allocate_bytes(sizeof(Literal));
   return new(bytes) Literal(alloc, type, value);
}

Node* Joos1WLexer::make_identifier(char const* name) {
   void* bytes = alloc.allocate_bytes(sizeof(Identifier));
   return new(bytes) Identifier(alloc, name);
}

Node* Joos1WLexer::make_modifier(Modifier::Type type) {
   void* bytes = alloc.allocate_bytes(sizeof(Modifier));
   return new(bytes) Modifier(type);
}

Node* Joos1WLexer::make_basic_type(BasicType::Type type) {
   void* bytes = alloc.allocate_bytes(sizeof(BasicType));
   return new(bytes) BasicType(type);
}
