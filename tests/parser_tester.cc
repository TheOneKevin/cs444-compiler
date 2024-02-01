#include <iostream>
#include <gtest/gtest.h>

#include "parser.h"
#include "lexer.h"

bool parse_grammar(
    const std::string& str
) {
    // Now that we have the string, lex it
    YY_BUFFER_STATE state;
    if (!(state = yy_scan_bytes(str.c_str(), str.size()))) {
        return false;
    }

    // Parse the tokens using Bison generated parser
    int what;
    parsetree::Node* parse_tree = nullptr;
    int result = yyparse(&what, &parse_tree);
    std::cout << "Parsing result: " << result;

    // Clean up Bison stuff
    yy_delete_buffer(state);

    // Now print the parse tree
    if (parse_tree) {
        std::cout << *parse_tree << std::endl;
        delete parse_tree;
    }

    return true;
}
