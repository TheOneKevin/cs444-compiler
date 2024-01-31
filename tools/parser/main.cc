#include <iostream>
#include <iterator>
#include <string>

#include "lexer.h"
#include "parser.h"

int main() {
    // Read entire input until EOF into string
    // For user interactivity, press CTRL+D to send EOF
    std::istreambuf_iterator<char> begin(std::cin), end;
    std::string str(begin, end);
    
    // Now that we have the string, lex it
    YY_BUFFER_STATE state;
    if (!(state = yy_scan_bytes(str.c_str(), str.size()))) {
        return false;
    }

    // Parse the tokens using Bison generated parser
    int what;
    parsetree::Node* parse_tree = nullptr;
    int result = yyparse(&what, &parse_tree);

    // Clean up Bison stuff
    yy_delete_buffer(state);

    // Now print the parse tree
    if (parse_tree) {
        std::cout << *parse_tree << std::endl;
        delete parse_tree;
    }

    return true;
}
