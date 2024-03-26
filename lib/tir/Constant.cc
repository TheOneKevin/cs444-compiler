#include "tir/Constant.h"

namespace tir {

ConstantInt* Constant::CreateBool(Context& ctx, bool value) {
   return ConstantInt::Create(ctx, Type::getInt1Ty(ctx), value);
}

} // namespace tir
