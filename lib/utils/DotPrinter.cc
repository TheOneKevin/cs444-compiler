#include "utils/DotPrinter.h"

namespace utils {

std::ostream& operator<<(std::ostream& os, DotPrinter::Sanitize s) {
   for(auto c : s.str) {
      switch(c) {
         case '<':
            os << "&lt;";
            break;
         case '>':
            os << "&gt;";
            break;
         case '"':
            os << "&quot;";
            break;
         case '\'':
            os << "&apos;";
            break;
         case '\n':
            if(s.left_align) {
               os << "<br align=\"left\"/>";
            } else {
               os << "<br/>";
            }
            break;
         case '&':
            os << "&amp;";
            break;
         default:
            // Check if c is a printable ASCII character
            if(c < 32 || c > 126) {
               os << "&#x" << std::hex << static_cast<int>(c) << ";";
            } else {
               os << c;
            }
            break;
      }
   }
   return os;
}

} // namespace utils
