#pragma once

#include <iostream>
#include <sstream>

/// @brief An opaque identifier representing a source file.
class FileId {

};

/// @brief A manager for source files.
class SourceManager {

};

/// @brief A specific location (line, column) in a source file.
class SourceLocation {
public:
   SourceLocation(FileId file, int line, int column)
         : file_{file}, line_{line}, column_{column} {}
   std::ostream& print(std::ostream& os) const {
      os << "??:" << line_ << ":" << column_;
      return os;
   }
   std::string toString() const {
      std::ostringstream os;
      print(os);
      return os.str();
   }
private:
   FileId file_;
   int line_;
   int column_;
};

class SourceRange {
public:
   SourceRange(SourceLocation begin, SourceLocation end)
         : begin_{begin}, end_{end} {}
   std::ostream& print(std::ostream& os) const {
      begin_.print(os);
      os << " - ";
      end_.print(os);
      return os;
   }
   std::string toString() const {
      std::ostringstream os;
      print(os);
      return os.str();
   }
private:
   SourceLocation begin_;
   SourceLocation end_;
};
