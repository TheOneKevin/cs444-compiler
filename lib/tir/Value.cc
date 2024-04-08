#include "tir/TIR.h"

namespace tir {

void Value::dump() const { print(std::cerr) << "\n"; }

} // namespace tir
