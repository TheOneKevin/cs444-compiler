#pragma once

#include <source_location>

#undef assert
#define assert(expr) \
   (static_cast<bool>(expr) ? void(0) : utils::_my_assert_fail(#expr))

namespace utils {

[[noreturn]]
void _my_assert_fail(
      const char* assertion,
      std::source_location location = std::source_location::current());

} // namespace utils
