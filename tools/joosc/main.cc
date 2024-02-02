#include <unistd.h>
#include <iostream>
#include <fstream>
#include <iterator>
#include <string>

#include "lexer.h"
#include "parser.h"

#include "utils/CommandLine.h"

int main(int argc, char **argv) {
    utils::InputParser input(argc, argv);
    std::istream *infile = &cin;
    struct cmd_error {};

    if (arc == 2) {
        infile = new ifstream( argv[3] );
    } else {
        cerr << "Usage: " << argv[0] << " input-file " << endl;
        exit( EXIT_FAILURE );
    }

    // Read entire input until enter
    std::string str;

    *infile << str;
    
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

    // Now print the parse tree
    if (parse_tree) {
        if(print_dot) {
            parsetree::print_dot(std::cout, *parse_tree);
        } else {
            std::cout << *parse_tree << std::endl;
        }
        delete parse_tree;
    }
      
    return 0;
}
