#include "utils/DotPrinter.h"

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
            os << "<br/>";
            break;
         case '&':
            os << "&amp;";
            break;
         default:
            os << c;
            break;
      }
   }
   return os;
}
