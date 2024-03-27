#include "tir/Context.h"

#include <iostream>

#include "tir/Constant.h"
#include "tir/Type.h"
#include "tir/Value.h"

namespace tir {

void Value::dump() const { print(std::cerr) << "\n"; }

std::ostream& operator<<(std::ostream& os, const Value& value) {
   return value.print(os);
}

Context::Context(BumpAllocator& alloc) : pimpl_{nullptr} {
   void* buf1 = alloc.allocate_bytes(sizeof(Type), alignof(Type));
   auto* pointerType = new(buf1) Type{this};
   void* buf2 = alloc.allocate_bytes(sizeof(Type), alignof(Type));
   auto* voidType = new(buf2) Type{this};
   void* buf3 = alloc.allocate_bytes(sizeof(Type), alignof(Type));
   auto* labelType = new(buf3) Type{this};
   void* buf4 = alloc.allocate_bytes(sizeof(ConstantNullPointer),
                                     alignof(ConstantNullPointer));
   auto* nullPointer = new(buf4) ConstantNullPointer{*this};
   // Replace pimpl_ with the new ContextPImpl
   void* buff = alloc.allocate_bytes(sizeof(ContextPImpl), alignof(ContextPImpl));
   pimpl_ = new(buff)
         ContextPImpl{&alloc, pointerType, voidType, labelType, nullPointer};
}

} // namespace tir