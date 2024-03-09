#include <algorithm>
#include <ranges>
#include <sstream>
#include <string_view>
#include <vector>

#include "diagnostics/Diagnostics.h"
#include "diagnostics/Location.h"
#include "diagnostics/SourceManager.h"

using namespace diagnostics;
using std::string_view;

/*

╭─[Error] initializer type must be assignable to declared type
╵  ╷
14 │  int i = (1==2);
   │  ~~~     ~~~~~~ this is Bool
╷  ╵  └> this is Int
╰─[/u/cs444/pub/assignment_testcases/a3/Je_6_Equality_int.java:14:6]

*/

/**
 * @brief Describes a highlight label, which is a section of the source line
 * (st, fn) plus a label (lbl).
 */
struct Highlight {
   int st, fn;
   std::string lbl;
};

/**
 * @brief Describes a line with multiple highlight labels
 */
struct Line {
   int lineNo;
   std::vector<Highlight> highlights;
};

/**
 * @brief Used to contain some state that can easily be passed around
 */
struct PrettyPrinter final {
   PrettyPrinter(SourceManager& SM, DiagnosticStorage const& DS)
         : SM(SM), DS(DS) {}

   // Main function to print the error message
   void printSingleError() {
      // The first element is always the location
      SourceRange curPos = std::get<SourceRange>(DS.args()[0]);
      file = curPos.range_start().file();
      msgs.emplace_back(curPos, std::ostringstream{});
      for(size_t i = 1; i < DS.args().size(); i++) {
         auto& arg = DS.args()[i];
         if(std::holds_alternative<SourceRange>(arg)) {
            curPos = std::get<SourceRange>(arg);
            msgs.emplace_back(curPos, std::ostringstream{});
         } else if(std::holds_alternative<string_view>(arg)) {
            msgs.back().second << std::get<string_view>(arg);
         } else {
            msgs.back().second << std::get<uint64_t>(arg);
         }
      }
      // Let's verify that the source ranges are sane i.e., source ranges
      // are on the same line, otherwise it'll fuck up rendering.
      bool isSane = true;
      for(auto& [pos, os] : msgs) {
         auto posStart = pos.range_start();
         auto posEnd = pos.range_end();
         isSane &= posStart.file() == posEnd.file();
         isSane &= posStart.line() == posEnd.line();
      }
      // If it's not sane, then print the message in a different way
      if(!isSane) {
         printInsaneError();
         return;
      }
      // We should sort the messages by position, but leave the first intact
      // The layout after sorting is: [
      //   { loc, main error message },
      //   { loc, first/leftmost label },
      //   ...
      //   { loc, last/rightmost label }
      // ]
      if(msgs.size() > 1) {
         std::sort(msgs.begin() + 1, msgs.end(), [](auto& a, auto& b) {
            return a.first.range_start().column() < b.first.range_start().column();
         });
      }
      // Now build the lines vector, iterate through msgs except the first one
      for(auto it = msgs.begin() + 1; it != msgs.end(); it++) {
         auto& [pos, os] = *it;
         auto lineNo = pos.range_start().line();
         auto& line = findOrCreateLine(lineNo);
         line.highlights.emplace_back(
               Highlight{static_cast<int>(pos.range_start().column()),
                         static_cast<int>(pos.range_end().column()),
                         os.str()});
      }
      // If there is only 1 message, add it to the line
      if(msgs.size() == 1) {
         auto& [pos, os] = msgs[0];
         auto lineNo = pos.range_start().line();
         auto& line = findOrCreateLine(lineNo);
         line.highlights.emplace_back(
               Highlight{static_cast<int>(pos.range_start().column()),
                         static_cast<int>(pos.range_end().column()),
                         ""});
      }
      // Compute the max line number size
      int maxDigits = 0;
      for(auto& line : lines)
         maxDigits = std::max(maxDigits, numdigits(line.lineNo));
      padding = std::string(maxDigits - 1, ' ');
      // Now print the error message
      std::ostringstream oss;
      auto posStart = std::get<SourceRange>(DS.args()[0]).range_start();
      oss << "╭─[Error] " << msgs[0].second.str() << "\n";
      oss << "╵" << padding << " ╷ \n";
      for(auto& line : lines) renderLine(oss, line);
      oss << "│\n"
          << "╰─[" << SM.getFileName(posStart.file()) << ":" << posStart.line()
          << ":" << posStart.column() << "]\n";
      // Print the error message
      std::cerr << oss.str();
   }

private:
   // Grabs the line from the source range and prints it to os
   std::ostream& printCodeLine(int line, std::ostream& os, int& skipped) {
      // Now we have to count the lines until we reach the correct one
      auto buf = SM.getBuffer(file);
      size_t i = 0;
      for(int l = 1; i < buf.length() && (l != line); i++) {
         if(buf[i] == '\n') l++;
      }
      // Skip whitespace at the beginning of the line
      while(i < buf.length() && (buf[i] == ' ' || buf[i] == '\t')) {
         skipped++;
         i++;
      }
      // Print the line until the end or until we reach the next line
      for(; i < buf.length() && buf[i] != '\n'; i++) {
         if(buf[i] == '\t')
            os << " ";
         else
            os << buf[i];
      }
      return os;
   }

   // Prints a bland unformatted error message
   void printInsaneError() { std::cerr << "Error: Insane source ranges\n"; }

   // Finds the Line struct with the given line number, or creates a new one
   Line& findOrCreateLine(int lineNo) {
      auto it = std::find_if(lines.begin(), lines.end(), [lineNo](auto& line) {
         return line.lineNo == lineNo;
      });
      if(it == lines.end()) {
         lines.emplace_back(Line{lineNo, {}});
         return lines.back();
      }
      return *it;
   }

   // Gets the number of digits in an integer
   int numdigits(int n) {
      int digits = 0;
      while(n) n /= 10, digits++;
      return digits;
   }

   void renderPadding(std::ostream& os, bool isLast = false) {
      if(isLast)
         os << "╷" << padding << " ╵ ";
      else
         os << padding << "  │ ";
   }

   // Renders a single Line struct
   void renderLine(std::ostream& os, Line& line) {
      // 0. Pad out line number
      auto lineStr = std::to_string(line.lineNo);
      lineStr.insert(lineStr.begin(), padding.length() + 1 - lineStr.size(), ' ');
      // 1. Render the source code line
      int skip = 0;
      os << lineStr << " │ ";
      printCodeLine(line.lineNo, os, skip) << "\n";
      // 2. Render the highlights layer, consisting of '~' where the each of the
      //    highlights start/end. Note, the highlights are sorted and do not
      //    overlap. Also print out the stems for the arrows.
      int col = skip + 1;
      std::string stems;
      renderPadding(os, line.highlights.size() < 2);
      for(auto& highlight : line.highlights) {
         for(; col < highlight.st; col++) {
            os << " ";
            stems += " ";
         }
         for(; col <= highlight.fn; col++) {
            if(col == highlight.st)
               stems += "1";
            else
               stems += " ";
            os << "~";
         }
      }
      // 3. Print the last highlight label, it's special
      if(!line.highlights.empty()) {
         auto& highlight = line.highlights.back();
         os << " " << highlight.lbl;
      }
      // 4. Next line and bail out if no more labels
      os << "\n";
      if(line.highlights.size() < 2) {
         return;
      }
      // 5. Print the highlights in reverse order, because we want to print
      //    the arrow stems from left to right. Now, if the current stem is
      //    at the position we want to print, emit an arrow head instead
      //    and print the label. Then bail out.
      auto highlights = line.highlights |
                        std::views::take(line.highlights.size() - 1) |
                        std::views::reverse;
      size_t nth = line.highlights.size() - 1;
      for(auto highlight : highlights) {
         renderPadding(os, nth == 1);
         size_t stemsSeen = 1;
         for(char c : stems) {
            if(c == ' ') {
               os << c;
            } else if(stemsSeen++ == nth) {
               os << "└> " << highlight.lbl;
               break;
            } else {
               os << "│";
            }
         }
         os << "\n";
         nth--;
      }
   }

private:
   SourceManager& SM;
   DiagnosticStorage const& DS;
   // A pair of { position, message } for the error message
   std::vector<std::pair<SourceRange, std::ostringstream>> msgs;
   // A vector of lines to render, after msgs have been processed
   std::vector<Line> lines;
   // Padding for the left margin
   std::string padding;
   // The source file this error is in
   SourceFile file;
};

void pretty_print_errors(SourceManager& SM, DiagnosticEngine& diag) {
   for(auto& diag : diag.errors()) {
      PrettyPrinter{SM, diag}.printSingleError();
   }
}
