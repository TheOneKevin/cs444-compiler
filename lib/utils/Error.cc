#include "utils/Error.h"

#ifdef ENABLE_STACKTRACE
#include <third-party/backward.h>
#endif


namespace utils {

std::string FatalError::get_trace(const std::string& what) {
#ifdef ENABLE_STACKTRACE
   using namespace backward;
   // Load trace from current location
   StackTrace stackTrace;
   TraceResolver resolver;
   stackTrace.load_here();
   resolver.load_stacktrace(stackTrace);
   std::ostringstream ss;
   // Print the stack trace
   Printer p;
   p.color_mode = ColorMode::always;
   p.inliner_context_size = 1;
   p.trace_context_size = 1;
   p.print(stackTrace, ss);
   // Print the error at the end for better visibility
   if(!what.empty())
      ss << "Error: " << what << std::endl;
   return ss.str();
#else
   return what;
#endif
}

std::string AssertError::get_trace(const std::string& what) {
#ifdef ENABLE_STACKTRACE
   using namespace backward;
   // Load trace from current location
   StackTrace stackTrace;
   TraceResolver resolver;
   stackTrace.load_here();
   stackTrace.skip_n_firsts(5);
   resolver.load_stacktrace(stackTrace);
   std::ostringstream ss;
   // Print the stack trace
   Printer p;
   p.color_mode = ColorMode::always;
   p.inliner_context_size = 1;
   p.trace_context_size = 1;
   p.print(stackTrace, ss);
   // Print the error at the end for better visibility
   ss << what;
   return ss.str();
#else
   return what;
#endif
}

} // namespace utils