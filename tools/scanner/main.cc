#include <unistd.h>
#include <iostream>

#include "lexer.h"
#include "parser.h"

extern std::string joos1w_parser_resolve_token (int yysymbol);

int main(void) {
    // Check if input is being piped in
    bool is_piped = !isatty(STDIN_FILENO);
    // Print the banner
    if(!is_piped) {
        std::cout << "     ______                          " << std::endl;
        std::cout << " __ / / __/______ ____  ___  ___ ____" << std::endl;
        std::cout << "/ // /\\ \\/ __/ _ `/ _ \\/ _ \\/ -_) __/" << std::endl;
        std::cout << "\\___/___/\\__/\\_,_/_//_/_//_/\\__/_/   " << std::endl;
        std::cout << "                                     " << std::endl;
        std::cout << "This is the Joos1W Scanner. Enter a string to be lexed followed by enter." << std::endl;
    }
    // If input is not being piped in, run once, then exit
    do {
        // Print the prompt, this will not be multiline (use pipe for that)
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

        // Lex the tokens using Flex generated lexer
        YYSTYPE lexval;
        YYLTYPE location;
        while(int lextok = yylex(&lexval, &location)) {
            std::cout
                << lextok << ": "
                << joos1w_parser_resolve_token(lextok)
                << std::endl;
        }

        // Clean up Bison stuff
        yy_delete_buffer(state);
    } while(!is_piped);
    return 0;
}
