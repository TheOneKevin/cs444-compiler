#include <unistd.h>
#include <iostream>
#include <fstream>
#include <iterator>
#include <fstream>
#include <sstream>

#include "ast/AST.h"
#include "lexer.h"
#include "parser.h"
#include "parsetree/ParseTreeTypes.h"
#include "parsetree/ParseTreeVisitor.h"

// FIXME(kevin): Remove when we have proper AST handling
bool isLiteralTypeValid(parsetree::Node* node) {
    if(node == nullptr) return true;
    if(node->get_node_type() == parsetree::Node::Type::Literal)
        return static_cast<parsetree::Literal*>(node)->isValid();
    for(size_t i = 0; i < node->num_children(); i++) {
        if(!isLiteralTypeValid(node->child(i)))
            return false;
    }
    return true;
}

int main(int argc, char **argv) {
    std::string str;
    struct cmd_error {};

    // Check for correct number of arguments
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " input-file " << std::endl;
        exit( EXIT_FAILURE );
    }
    
    // Check if the file is a .java file
    std::string filePath = argv[1];
    std::string fileName = filePath.substr(filePath.find_last_of("/\\") + 1);
    if(filePath.substr(filePath.length() - 5, 5) != ".java") {
        std::cerr << "Error: not a valid .java file" << std::endl;
        return 42;
    }

    // Read file from command-line argument
    try {
        // Read entire input
        std::ifstream inputFile(filePath);
        std::stringstream buffer;
        buffer << inputFile.rdbuf();
        str = buffer.str();
    } catch ( ... ) {
        std::cerr << "Error! Could not open input file \"" << filePath << "\"" << std::endl;
        exit( EXIT_FAILURE );
    }

    // Check for non-ASCII characters
    for(unsigned i = 0; i < str.length(); i++) {
        if (static_cast<unsigned char>(str[i]) > 127) {
            std::cerr << "Parse error: non-ASCII character in input" << std::endl;
            return 42;
        }
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

    // Check if the parse tree is poisoned
    if(parse_tree->is_poisoned()) {
        std::cerr << "Parse error: parse tree is poisoned" << std::endl;
        return 42;
    }

    // Check if the parse tree has valid literal types
    if(!isLiteralTypeValid(parse_tree)) {
        std::cerr << "Parse error: invalid literal type" << std::endl;
        return 42;
    }

    // Build the AST from the parse tree
    struct parse_error {};
    ast::CompilationUnit* ast = nullptr;
    try {
        if(parse_tree->is_poisoned()) throw parse_error();
        ast = parsetree::visitCompilationUnit(parse_tree);
    } catch(const std::runtime_error& re) {
        std::cerr << "Runtime error: " << re.what() << std::endl;
        return 42;
    } catch(...) {
        std::cerr << "Unknown failure occurred." << std::endl;
        return 1;
    }
    if (!ast) return 42;

    // Check the AST to make sure the class/intf has the same name as file
    auto cuBody = dynamic_cast<ast::Decl*>(ast->getBody());
    fileName = fileName.substr(0, fileName.length() - 5);
    if (cuBody->getName() != fileName) {
        std::cerr << "Parse error: class/interface name does not match file name" << std::endl;
        std::cerr << "Class/interface name: " << cuBody->getName() << std::endl;
        std::cerr << "File name: " << fileName << std::endl;
        return 42;
    }

    // Clean up the parse tree
    delete parse_tree;
    return 0;
}
