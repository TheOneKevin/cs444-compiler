#pragma once

#include <FlexLexer.h>
#define INCLUDED_FLEXLEXER_H
#include "grammar/joos1w_lexer_internal.h"
#undef INCLUDED_FLEXLEXER_H

#include <iostream>
#include <memory_resource>
#include <sstream>
#include <vector>

class Joos1WParser final {
public:
   Joos1WParser(std::string const& in, parsetree::BumpAllocator& alloc)
         : lexer{alloc}, iss{in} {
      buffer = lexer.yy_create_buffer(iss, in.size());
      lexer.yy_switch_to_buffer(buffer);
   }

   Joos1WParser(std::string const& in)
         : mbr{}, alloc{&mbr}, lexer{alloc}, iss{in} {
      buffer = lexer.yy_create_buffer(iss, in.size());
      lexer.yy_switch_to_buffer(buffer);
   }

   int yylex() { return lexer.yylex(); }

   int parse(parsetree::Node*& ret) {
      ret = nullptr;
      return yyparse(&ret, lexer);
   }

   ~Joos1WParser() {
      if(buffer) lexer.yy_delete_buffer(buffer);
   }

private:
   std::pmr::monotonic_buffer_resource mbr;
   parsetree::BumpAllocator alloc;
   Joos1WLexer lexer;
   yy_buffer_state* buffer;
   std::istringstream iss;
};
