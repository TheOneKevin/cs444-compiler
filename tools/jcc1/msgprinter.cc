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

#ifdef COLORS
static constexpr char const* RED_BOLD = "\x1b[1;31m";
static constexpr char const* BLUE = "\x1b[0;94m";
static constexpr char const* MAGENTA = "\x1b[0;35m";
static constexpr char const* RESET = "\x1b[0m";
#else
static constexpr char const* RED_BOLD = "";
static constexpr char const* BLUE = "";
static constexpr char const* MAGENTA = "";
static constexpr char const* RESET = "";
#endif

// Gets the number of digits in an integer
static int numdigits(int n) {
   int digits = 0;
   while(n) n /= 10, digits++;
   return digits;
}

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
   SourceFile file;
   std::vector<Highlight> highlights;

   // Sorts the highlights by start position
   void sortHighlights() {
      std::sort(highlights.begin(), highlights.end(), [](auto& a, auto& b) {
         return a.st < b.st;
      });
   }
};

/**
 * @brief Describes a source file with multiple lines and highlights
 */
struct File {
   SourceFile file;
   std::vector<Line> lines;

   // Finds the Line struct with the given line number, or creates a new one
   Line& findOrCreateLine(int lineNo) {
      auto it = std::find_if(lines.begin(), lines.end(), [lineNo](auto& line) {
         return line.lineNo == lineNo;
      });
      if(it == lines.end()) {
         lines.emplace_back(Line{lineNo, file, {}});
         return lines.back();
      }
      return *it;
   }

   // Sorts the highlights for each line
   void sortHighlights() {
      // Sort the highlights for each line
      for(auto& line : lines) line.sortHighlights();
      // Now sort the lines by line number
      std::sort(lines.begin(), lines.end(), [](auto& a, auto& b) {
         return a.lineNo < b.lineNo;
      });
   }

   // Returns the maximum number of digits in the line numbers
   int maxDigits() {
      int maxDigits = 0;
      for(auto& line : lines)
         maxDigits = std::max(maxDigits, numdigits(line.lineNo));
      return maxDigits;
   }
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
      // Now build the lines vector, iterate through msgs except the first one
      for(auto it = msgs.begin() + 1; it != msgs.end(); it++) {
         auto& [pos, os] = *it;
         auto lineNo = pos.range_start().line();
         auto& file = findOrCreateFile(pos.range_start().file());
         auto& line = file.findOrCreateLine(lineNo);
         line.highlights.emplace_back(
               Highlight{static_cast<int>(pos.range_start().column()),
                         static_cast<int>(pos.range_end().column()),
                         os.str()});
      }
      // If there is only 1 message, add it to the line
      if(msgs.size() == 1) {
         auto& [pos, os] = msgs[0];
         auto lineNo = pos.range_start().line();
         auto& file = findOrCreateFile(pos.range_start().file());
         auto& line = file.findOrCreateLine(lineNo);
         line.highlights.emplace_back(
               Highlight{static_cast<int>(pos.range_start().column()),
                         static_cast<int>(pos.range_end().column()),
                         ""});
      }
      // Compute the max gutter width
      int maxDigits = 0;
      for(auto& file : files) maxDigits = std::max(maxDigits, file.maxDigits());
      padding = std::string(maxDigits, ' ');
      // Now print the error message
      std::ostringstream oss;
      auto posStart = std::get<SourceRange>(DS.args()[0]).range_start();
      oss << "╭─[" << RED_BOLD << "Error" << RESET << "] " << msgs[0].second.str()
          << "\n";
      for(auto& file : files) renderFile(oss, file, files.size() == 1);
      if(files.size() > 1) oss << "│\n";
      oss << "╰─[" << BLUE << SM.getFileName(posStart.file()) << ":"
          << posStart.line() << ":" << posStart.column() << RESET << "]\n";
      // Print the error message
      std::cerr << oss.str();
   }

private:
   // Grabs the line from the source range and prints it to os
   std::ostream& printCodeLine(Line ll, std::ostream& os, int& skipped) {
      auto buf = SM.getBuffer(ll.file);
      size_t i = 0;
      // Count the lines until we reach the correct one
      for(int l = 1; i < buf.length() && (l != ll.lineNo); i++) {
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
   void printInsaneError() { std::cerr << "Error: Insane source ranges"; }

   // Finds the File struct with the given source file, or creates a new one
   File& findOrCreateFile(SourceFile file) {
      auto it = std::find_if(files.begin(), files.end(), [file](auto& f) {
         return f.file == file;
      });
      if(it == files.end()) {
         files.emplace_back(File{file, {}});
         return files.back();
      }
      return *it;
   }

   void renderGutter(std::ostream& os, int lineNo = -1) {
      if(lineNo > 0) {
         // Pad the line number with spaces
         auto str = std::to_string(lineNo);
         str.insert(str.begin(), padding.length() - str.size(), ' ');
         // Render it in the gutter
         os << "│ " << BLUE << str << RESET << " │ ";
      } else {
         // There is no line number, just render the padding
         os << "│ " << padding << " │ ";
      }
   }

   // Renders a single File struct
   void renderFile(std::ostream& os, File& file, bool isOnly) {
      // 1. Sort the highlights for each line
      file.sortHighlights();
      // 2. Render each line
      os << "│ " << padding << " ╷\n";
      for(auto& line : file.lines) renderLine(os, line);
      os << "│ " << padding << " ╵\n";
      // 3. If this is not the only file, print the file name
      if(!isOnly) os << "├─(in " << SM.getFileName(file.file) << ")\n";
   }

   // Renders a single Line struct
   void renderLine(std::ostream& os, Line& line) {
      // 1. Render the source code line
      int skip = 0;
      renderGutter(os, line.lineNo);
      printCodeLine(line, os, skip) << "\n";
      // 2. Render the highlights layer, consisting of '~' where the each of the
      //    highlights start/end. Note, the highlights are sorted and do not
      //    overlap. Also print out the stems for the arrows.
      int col = skip + 1;
      std::string stems;
      renderGutter(os);
      os << MAGENTA;
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
      os << RESET;
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
         renderGutter(os);
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
   // A vector of files to render, after msgs have been processed
   std::vector<File> files;
   // Padding for the gutter
   std::string padding;
};

void pretty_print_errors(SourceManager& SM, DiagnosticEngine& diag) {
   std::cerr << "\n";
   for(auto& diag : diag.errors()) {
      PrettyPrinter{SM, diag}.printSingleError();
      std::cerr << "\n";
   }
   for(auto& diag : diag.warnings()) {
      PrettyPrinter{SM, diag}.printSingleError();
      std::cerr << "\n";
   }
}
