#pragma once

#include <cassert>
#include <iostream>
#include <sstream>
#include <fstream>

class SourceLocation;
class SourceRange;
class SourceFile;

/// @brief An opaque identifier representing a source file.
class SourceFile {
public:
   /// @brief Construct a new SourceFile with no associated file.
   SourceFile() : id_{-1} {}

   bool operator==(SourceFile const& other) const { return id_ == other.id_; }

private:
   int id_;
};

/// @brief A specific location (line, column) in a source file.
class SourceLocation {
   friend class SourceRange;
   SourceLocation() : file_{}, line_{-1}, column_{-1} {}

public:
   SourceLocation(SourceFile file, int line, int column)
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

   /// @brief Returns true if the SourceLocation was not default constructed. 
   bool isValid() const { return line_ != -1; }

private:
   SourceFile file_;
   int line_;
   int column_;
};

/// @brief A range of locations in a source file.
class SourceRange {
public:
   /// @brief Construct a new SourceRange with no associated file.
   SourceRange() : begin_{}, end_{} {}

   /// @brief Construct a new SourceRange with the given begin and end
   /// locations.
   SourceRange(SourceLocation begin, SourceLocation end)
         : begin_{begin}, end_{end} {
      assert(begin.file_ == end.file_ && "SourceRange spans multiple files");
   }

   /// @brief Returns true if the SourceRange was not default constructed.
   /// Checks if each SourceLocation is valid.
   bool isValid() const { return begin_.isValid() && end_.isValid(); }

   std::ostream& print(std::ostream& os) const {
      os << "??:" << begin_.line_ << ":" << begin_.column_ << " - " << end_.line_
         << ":" << end_.column_;
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
