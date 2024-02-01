#include "AST.h"
#include "../parsetree/ParseTreeVisitor.h"

#include <unistd.h>
#include <iostream>
#include <iterator>
#include <string>

#include "lexer.h"
#include "parser.h"

#include "utils/CommandLine.h"

int main(int argc, char **argv) {

    // Read entire input until enter
    std::istreambuf_iterator<char> eos;
    std::string str = std::string(std::istreambuf_iterator<char>(std::cin), eos);
    
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
    
    parsetree::visitCompilationUnit(parse_tree);
    return 0;
}
