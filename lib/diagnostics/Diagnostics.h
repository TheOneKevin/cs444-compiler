#pragma once

#include <exception>
#include <forward_list>
#include <iostream>
#include <ranges>
#include <string>
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
         : ranges{loc, SourceRange{}, SourceRange{}} {}

   void addArgument(std::string_view arg) {
      assert(argIndex < MaxArguments && "too many arguments");
      arguments[argIndex++] = arg;
   }

   void addArgument(uint64_t arg) {
      assert(argIndex < MaxArguments && "too many arguments");
      arguments[argIndex++] = arg;
   }

   void addRange(SourceRange range) {
      assert(rangeIndex < MaxSourceRanges && "too many ranges");
      ranges[rangeIndex++] = range;
   }

   std::ostream& emit(std::ostream& os) const {
      for(auto& range : ranges) {
         if(range.isValid()) {
            range.print(os);
         }
      }
      os << ": ";
      for(auto& arg : arguments) {
         if(std::holds_alternative<std::string_view>(arg)) {
            os << std::get<std::string_view>(arg);
         } else {
            os << std::get<uint64_t>(arg);
         }
      }
      return os;
   }

private:
   int argIndex = 0;
   int rangeIndex = 0;

   static constexpr int MaxSourceRanges = 3;
   SourceRange ranges[MaxSourceRanges];

   static constexpr int MaxArguments = 10;
   std::variant<std::string_view, uint64_t> arguments[MaxArguments];
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
// DiagnosticEngine
/* ===--------------------------------------------------------------------=== */

class DiagnosticEngine {
public:
   DiagnosticEngine() = default;
   DiagnosticBuilder ReportError(SourceRange loc) {
      diagnostics_.emplace_after(diagnostics_.before_begin(), loc);
      return DiagnosticBuilder{diagnostics_.front()};
   }
   bool hasErrors() const { return !diagnostics_.empty(); }
   auto messages() const { return std::views::all(diagnostics_); }

private:
   std::forward_list<DiagnosticStorage> diagnostics_;
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
static inline DiagnosticBuilder& operator<<(DiagnosticBuilder&& builder,
                                            T value) {
   return (DiagnosticBuilder&)builder << value;
}

} // namespace diagnostics