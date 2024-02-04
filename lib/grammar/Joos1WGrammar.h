#pragma once

#include <FlexLexer.h>
#define INCLUDED_FLEXLEXER_H
#include "grammar/joos1w_lexer_internal.h"
#undef INCLUDED_FLEXLEXER_H

#include <iostream>
#include <sstream>
#include "parsetree/ParseTree.h"

class Joos1WParser final {
public:
    Joos1WParser(std::istream& in) : lexer{}, buffer{nullptr} {
        // FIXME(kevin): Why are we redirecting to stderr?
        lexer.switch_streams(in, std::cerr);
    }

    Joos1WParser(std::string const& in) : lexer{} {
        std::istringstream iss{in};
        buffer = lexer.yy_create_buffer(&iss, in.size());
        lexer.yy_switch_to_buffer(buffer);
    }

    int yylex() {
        return lexer.yylex();
    }

    int parse(parsetree::Node* &ret) {
        ret = nullptr;
        return yyparse(&ret, lexer);
    }

private:
    Joos1WLexer lexer;
    yy_buffer_state* buffer;
};
