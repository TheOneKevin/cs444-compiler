#include <unistd.h>
#include <iostream>
#include <fstream>
#include <iterator>
#include <fstream>
#include <sstream>

#include "ast/AST.h"
#include "lexer.h"
#include "parser.h"
#include "parsetree/ParseTreeVisitor.h"

int main(int argc, char **argv) {
    std::string str;
    struct cmd_error {};

    // Read file from command-line
    if (argc == 2) {
        try {
            // Read entire input
            std::ifstream inputFile(argv[1]);
            std::stringstream buffer;
            buffer << inputFile.rdbuf();
            str = buffer.str();
        } catch ( ... ) {
            std::cerr << "Error! Could not open input file \"" << argv[1] << "\"" << std::endl;
            exit( EXIT_FAILURE );
        }
    } else {
        std::cerr << "Usage: " << argv[0] << " input-file " << std::endl;
        exit( EXIT_FAILURE );
    }
    
    // Now that we have the string, lex it
    YY_BUFFER_STATE state;
    if (!(state = yy_scan_bytes(str.c_str(), str.size()))) {
        return 1;
    }

    // Parse the tokens using Bison generated parser
    int what;
    parsetree::Node* parse_tree = nullptr;
    int result = yyparse(&what, &parse_tree);

    // Clean up Bison stuff
    yy_delete_buffer(state);

    if (!parse_tree || result) {
        return 42;
    }

    struct parse_error {};

    try {
        if(parse_tree->is_poisoned()) throw parse_error();
        parsetree::visitCompilationUnit(parse_tree);
    } catch ( ... ) {
        // output some error
        return 42;
    }

    delete parse_tree;
    return 0;
}
