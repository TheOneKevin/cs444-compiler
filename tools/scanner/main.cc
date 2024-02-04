#include <unistd.h>
#include <iostream>
#include <sstream>

#include "grammar/Joos1WGrammar.h"

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
            str = std::string{std::istreambuf_iterator<char>(std::cin), eos};
        }
        
        // Lex the tokens using Flex generated lexer
        Joos1WParser parser{str};
        while(int lextok = parser.yylex()) {
            std::cout
                << lextok << ": "
                << joos1w_parser_resolve_token(lextok)
                << std::endl;
        }
    } while(!is_piped);
    return 0;
}
