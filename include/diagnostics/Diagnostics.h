#pragma once

#include <forward_list>
#include <iostream>
#include <ranges>
#include <string_view>
#include <variant>

#include "diagnostics/Location.h"

namespace diagnostics {

class DiagnosticStorage;
class DiagnosticBuilder;
class DiagnosticEngine;

/* ===--------------------------------------------------------------------=== */
// DiagnosticStorage
/* ===--------------------------------------------------------------------=== */

class DiagnosticStorage {
   friend class DiagnosticEngine;

public:
   DiagnosticStorage(SourceRange loc)
         : argIndex{1}, arguments_{loc} {}

   void addArgument(std::string_view arg) {
      assert(argIndex < MaxArguments && "too many arguments");
      arguments_[argIndex++] = arg;
   }

   void addArgument(uint64_t arg) {
      assert(argIndex < MaxArguments && "too many arguments");
      arguments_[argIndex++] = arg;
   }

   void addRange(SourceRange range) {
      assert(argIndex < MaxArguments && "too many ranges");
      arguments_[argIndex++] = range;
   }

   auto args() const { return std::views::counted(arguments_, argIndex); }

   std::ostream& emit(std::ostream& os) const {
      for(auto& arg : arguments_) {
         if(std::holds_alternative<std::string_view>(arg)) {
            os << std::get<std::string_view>(arg);
         } else if(std::holds_alternative<SourceRange>(arg)) {
            os << "\n\tat:";
            std::get<SourceRange>(arg).print(os);
         } else {
            os << std::get<uint64_t>(arg);
         }
      }
      return os;
   }

private:
   int argIndex = 0;
   static constexpr int MaxArguments = 15;
   std::variant<std::string_view, uint64_t, SourceRange> arguments_[MaxArguments];
};

/* ===--------------------------------------------------------------------=== */
// DiagnosticBuilder
/* ===--------------------------------------------------------------------=== */

class DiagnosticBuilder {
public:
   DiagnosticBuilder(DiagnosticStorage& loc) noexcept : storage{loc} {}
   DiagnosticStorage& storage;
};

/* ===--------------------------------------------------------------------=== */
// DiagnosticStream
/* ===--------------------------------------------------------------------=== */

class DiagnosticStream : public std::ostream {
public:
   explicit DiagnosticStream(std::ostream& stream) : stream_{stream} {}
   std::ostream& get() { return buffer; }
   ~DiagnosticStream() {
      auto str = buffer.str();
      stream_ << str;
      // Prevent double newlines
      if(!str.ends_with('\n')) stream_ << std::endl;
   }

private:
   std::ostream& stream_;
   std::ostringstream buffer;
};

template <typename T>
DiagnosticStream& operator<<(DiagnosticStream& str, T&& value) {
   str.get() << value;
   return str;
}

/* ===--------------------------------------------------------------------=== */
// DiagnosticEngine
/* ===--------------------------------------------------------------------=== */

class DiagnosticEngine {
public:
   explicit DiagnosticEngine(int verbose = 0) : verbose_{verbose} {}
   DiagnosticBuilder ReportError(SourceRange loc) {
      errors_.emplace_after(errors_.before_begin(), loc);
      return DiagnosticBuilder{errors_.front()};
   }
   DiagnosticBuilder ReportWarning(SourceRange loc) {
      warnings_.emplace_after(warnings_.before_begin(), loc);
      return DiagnosticBuilder{warnings_.front()};
   }
   DiagnosticStream ReportDebug(int level = 1) {
      assert(Verbose(level) &&
             "Debug messages not available. Did you forget to check for Verbose?");
      // FIXME(kevin): In the future, allow for custom streams
      return DiagnosticStream{std::cerr};
   }
   void setVerbose(int verbose) { verbose_ = verbose; }
   bool hasErrors() const { return !errors_.empty(); }
   auto errors() const { return std::views::all(errors_); }
   bool hasWarnings() const { return !warnings_.empty(); }
   auto warnings() const { return std::views::all(warnings_); }

public:
   bool Verbose(int level = 1) const { return verbose_ >= level; }

private:
   int verbose_ = 0;
   std::forward_list<DiagnosticStorage> errors_;
   std::forward_list<DiagnosticStorage> warnings_;
};

/* ===--------------------------------------------------------------------=== */
// Stream operators for DiagnosticBuilder
/* ===--------------------------------------------------------------------=== */

static inline DiagnosticBuilder& operator<<(DiagnosticBuilder& builder,
                                            std::string_view str) {
   builder.storage.addArgument(str);
   return builder;
}

static inline DiagnosticBuilder& operator<<(DiagnosticBuilder& builder,
                                            const char* str) {
   builder.storage.addArgument(std::string_view{str});
   return builder;
}

static inline DiagnosticBuilder& operator<<(DiagnosticBuilder& builder,
                                            SourceRange range) {
   builder.storage.addRange(range);
   return builder;
}

template <typename T>
static inline DiagnosticBuilder& operator<<(DiagnosticBuilder&& builder, T value) {
   return (DiagnosticBuilder&)builder << value;
}

} // namespace diagnostics
