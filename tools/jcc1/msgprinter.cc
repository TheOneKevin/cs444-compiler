#include <sstream>
#include <string_view>

#include "diagnostics/Diagnostics.h"
#include "diagnostics/SourceManager.h"

using namespace diagnostics;
using std::string_view;

static void print_line_from_buf(string_view buf, size_t line,
                                std::ostringstream& os) {
   size_t i = 0;
   for(size_t l = 1; i < buf.length() && (l != line); i++) {
      if(buf[i] == '\n') l++;
   }
   for(; i < buf.length() && buf[i] != '\n'; i++) {
      if(buf[i] == '\t')
         os << " ";
      else
         os << buf[i];
   }
}

static void print_single_error(SourceManager& SM, DiagnosticStorage const& msg) {
   // Grab the file and line number
   auto main_loc = msg.ranges()[0];
   auto file_loc = main_loc.range_start().file();
   auto stLine = main_loc.range_start().line();
   auto fnLine = main_loc.range_end().line();
   auto stCol = main_loc.range_start().column();
   auto fnCol = main_loc.range_end().column();
   std::string fileName = SM.getFileName(file_loc);
   string_view fileBuf = SM.getBuffer(file_loc);
   // Search for the line
   std::ostringstream os;
   std::ostringstream os1;
   print_line_from_buf(fileBuf, stLine, os1);
   // Print the line number to a string
   std::string lineStr = std::to_string(stLine);
   std::string padding = std::string(lineStr.length()-1, ' ');
   // Print the message
   os << "╭─[Error] ";
   for(auto& arg : msg.args()) {
      if(std::holds_alternative<string_view>(arg)) {
         os << std::get<string_view>(arg);
      } else {
         os << std::get<uint64_t>(arg);
      }
   }
   os << "\n";
   os << "╵" << padding << " ╷\n";
   // Print the error line
   os << lineStr << " │ " << os1.str() << "\n";
   // If the line is the same, print the error marker
   if(stLine == fnLine) {
      os << "╷" << padding << " ╵ " << std::string(stCol - 1, ' ')
         << std::string(fnCol - stCol + 1, '~') << "\n";
   }
   // Print the file location
   os << "│\n";
   os << "╰─[" << fileName << ":" << stLine << ":" << stCol << "]\n";
   // Print to stderr
   std::cerr << os.str();
}

void pretty_print_errors(SourceManager& SM, DiagnosticEngine& diag) {
   for(auto& diag : diag.errors()) {
      print_single_error(SM, diag);
   }
}
