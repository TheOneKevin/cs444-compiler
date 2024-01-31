#include <unistd.h>
#include <iostream>
#include <iterator>
#include <string>

#include "lexer.h"
#include "parser.h"

int main() {
    // Check if input is being piped in
    bool is_piped = !isatty(STDIN_FILENO);
    // Print the banner
    if(!is_piped) {
        std::cout << "________________                                  " << std::endl;
        std::cout << "______  /__  __ \\_____ ___________________________" << std::endl;
        std::cout << "___ _  /__  /_/ /  __ `/_  ___/_  ___/  _ \\_  ___/" << std::endl;
        std::cout << "/ /_/ / _  ____// /_/ /_  /   _(__  )/  __/  /    " << std::endl;
        std::cout << "\\____/  /_/     \\__,_/ /_/    /____/ \\___//_/     " << std::endl;
        std::cout << "This is the Joos1W Parser. Enter a string to be parsed followed by enter." << std::endl;
    }
    // If input is not being piped in, run once, then exit
    do {
        // Print the prompt
        if(!is_piped) {
            std::cout << "> ";
        }

        // Read entire input until enter
        std::string str;
        if(!is_piped) {
            std::getline(std::cin, str);
        } else {
            std::istreambuf_iterator<char> eos;
            str = std::string(std::istreambuf_iterator<char>(std::cin), eos);
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
        std::cout << "Result: " << result << std::endl;

        // Clean up Bison stuff
        yy_delete_buffer(state);

        // Now print the parse tree
        if (parse_tree) {
            std::cout << *parse_tree << std::endl;
            delete parse_tree;
        }
    } while(!is_piped);
    return 0;
}