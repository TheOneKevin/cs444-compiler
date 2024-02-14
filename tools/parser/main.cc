#include <unistd.h>

#include <iostream>
#include <iterator>
#include <sstream>
#include <string>

#include "grammar/Joos1WGrammar.h"
#include "utils/CommandLine.h"

int main(int argc, char** argv) {
   utils::InputParser input(argc, argv);

   // Flag to enable bison debug
   if(input.cmdOptionExists("-d")) {
      yydebug = 1;
   } else {
      yydebug = 0;
   }

   // Flag to print the parse tree in dot format
   bool print_dot = false;
   if(input.cmdOptionExists("-x")) {
      print_dot = true;
   }

   // Check if input is being piped in
   bool is_piped = !isatty(STDIN_FILENO);
   // Print the banner
   if(!is_piped) {
      std::cout << "________________                                  "
                << std::endl;
      std::cout << "______  /__  __ \\_____ ___________________________"
                << std::endl;
      std::cout << "___ _  /__  /_/ /  __ `/_  ___/_  ___/  _ \\_  ___/"
                << std::endl;
      std::cout << "/ /_/ / _  ____// /_/ /_  /   _(__  )/  __/  /    "
                << std::endl;
      std::cout << "\\____/  /_/     \\__,_/ /_/    /____/ \\___//_/     "
                << std::endl;
      std::cout << "This is the Joos1W Parser. Enter a string to be parsed "
                   "followed by enter."
                << std::endl;
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

      // Parse the input
      diagnostics::DiagnosticEngine diag;
      Joos1WParser parser{str, &diag};
      parsetree::Node* parse_tree = nullptr;
      int result = parser.parse(parse_tree);
      if(!is_piped) {
         std::cout << "Result: " << result << std::endl;
      }

      if(diag.hasErrors()) {
         for(auto& msg : diag.messages()) {
            msg.emit(std::cerr) << std::endl;
         }
      }

      // Now print the parse tree
      if(parse_tree) {
         if(print_dot) {
            parse_tree->printDot(std::cout);
         } else {
            std::cout << *parse_tree << std::endl;
         }
      }

      if(is_piped) {
         return result;
      }
   } while(!is_piped);
   return 0;
}
