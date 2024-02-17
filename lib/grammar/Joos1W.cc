#include "grammar/Joos1WGrammar.h"
#include "parsetree/ParseTree.h"

using Node = parsetree::Node;
using Operator = parsetree::Operator;
using Literal = parsetree::Literal;
using Identifier = parsetree::Identifier;
using Modifier = parsetree::Modifier;
using BasicType = parsetree::BasicType;

Node* Joos1WLexer::make_poison(YYLTYPE& loc) {
   void* bytes = alloc.allocate_bytes(sizeof(Node));
   return new(bytes) Node(make_range(loc), Node::Type::Poison);
}

Node* Joos1WLexer::make_operator(YYLTYPE& loc, Operator::Type type) {
   void* bytes = alloc.allocate_bytes(sizeof(Operator));
   return new(bytes) Operator(make_range(loc), type);
}

Node* Joos1WLexer::make_literal(YYLTYPE& loc, Literal::Type type,
                                char const* value) {
   void* bytes = alloc.allocate_bytes(sizeof(Literal));
   return new(bytes) Literal(make_range(loc), alloc, type, value);
}

Node* Joos1WLexer::make_identifier(YYLTYPE& loc, char const* name) {
   void* bytes = alloc.allocate_bytes(sizeof(Identifier));
   return new(bytes) Identifier(make_range(loc), alloc, name);
}

Node* Joos1WLexer::make_modifier(YYLTYPE& loc, Modifier::Type type) {
   void* bytes = alloc.allocate_bytes(sizeof(Modifier));
   return new(bytes) Modifier(make_range(loc), type);
}

Node* Joos1WLexer::make_basic_type(YYLTYPE& loc, BasicType::Type type) {
   void* bytes = alloc.allocate_bytes(sizeof(BasicType));
   return new(bytes) BasicType(make_range(loc), type);
}
