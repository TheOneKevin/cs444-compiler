#pragma once

#include <iostream>
#include <list>
#include <ranges>
#include <string>
#include <string_view>
#include <fstream>
#include <utils/Error.h>

class SourceManager;
class SourceLocation;
class SourceRange;

/* ===--------------------------------------------------------------------=== */
// SourceFile
/* ===--------------------------------------------------------------------=== */

/// @brief An opaque identifier representing a source file.
class SourceFile {
public:
   /// @brief Construct a new SourceFile with no associated file.
   SourceFile() : id_{nullptr} {}

   bool operator==(SourceFile const& other) const { return id_ == other.id_; }

private:
   explicit SourceFile(void const* ptr) : id_{ptr} {}

   friend class SourceManager;
   friend class SourceLocation;
   friend class SourceRange;

private:
   /// @brief The unique identifier for this source file.
   /// This is used to index into the SourceManager's file list.
   void const* id_;
};

/* ===--------------------------------------------------------------------=== */
// SourceManager
/* ===--------------------------------------------------------------------=== */

class SourceManager {
public:
   SourceManager() = default;
   SourceManager(SourceManager const&) = delete;
   SourceManager& operator=(SourceManager const&) = delete;

   /// @brief Add a file and its contents to the SourceManager.
   /// @param path The path to the file
   void addFile(std::string_view path) {
      // Check the path ends in ".java"
      if(path.size() < 5 || path.substr(path.size() - 5) != ".java") {
         throw utils::FatalError{"File " + std::string{path} + " is not a .java file"};
      }
      std::ifstream file{std::string{path}};
      if(!file) {
         throw utils::FatalError{"File " + std::string{path} + " does not exist"};
      }
      files_.emplace_back(path, std::istreambuf_iterator<char>{file}, this);
   }

   /// @brief Push a new buffer onto the buffer stack.
   void emplaceBuffer() {
      auto name = "Buffer " + std::to_string(files_.size() + 1);
      files_.emplace_back(name, this);
   }

   /// @brief Grab a reference to the current buffer.
   std::string& currentBuffer() { return files_.back().buffer; }

   /// @brief Get iterator for files
   auto files() const {
      return files_ | std::views::all |
             std::views::transform([](File const& file) -> SourceFile {
                return SourceFile{static_cast<File const*>(&file)};
             });
   }

   /// @brief Get the name of a file
   static std::string getFileName(SourceFile file) {
      auto* f = static_cast<File const*>(file.id_);
      if(f == nullptr || !f->isFile)
         return "";
      return f->name;
   }

   /// @brief Print the name of the file to the output stream.
   /// @param os The output stream to print to
   /// @param file The SourceFile to print
   static void print(std::ostream& os, SourceFile const& file) {
      if(file.id_ == nullptr) {
         os << "??";
      } else {
         os << static_cast<File const*>(file.id_)->name;
      }
   }

   /// @brief Get the buffer for a file
   /// @param file The file to get the buffer for
   /// @return A string_view of the buffer
   static std::string const& getBuffer(SourceFile file) {
      return static_cast<File const*>(file.id_)->buffer;
   }

private:
   struct File {
      std::string name;
      std::string buffer;
      bool isFile = false;
      SourceManager* parent;
      File(std::string_view name, std::istreambuf_iterator<char> begin,
           SourceManager* parent)
            : name{name}, buffer{begin, {}}, isFile{true}, parent{parent} {}
      File(std::string_view name, SourceManager* parent)
            : name{name}, buffer{}, parent{parent} {}
   };

   std::list<File> files_;
};
