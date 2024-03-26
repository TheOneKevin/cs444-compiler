#include "tir/Type.h"

namespace tir {
Type* Type::getInt1Ty(Context& ctx) { return IntegerType::get(ctx, 1); }
} // namespace tir
