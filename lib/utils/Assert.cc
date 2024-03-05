#include "utils/Assert.h"

#include <utils/Error.h>

namespace utils {

void _my_assert_fail(const char* assertion, std::source_location location) {
   // clang-format off
   throw utils::AssertError(
      "\n"
      "Assertion failed: " + std::string(assertion) + "\n" +
      "              at: " + location.file_name() + ":" + std::to_string(location.line()) + "\n" +
      "        function: " + location.function_name()
   );
   // clang-format on
}

} // namespace utils