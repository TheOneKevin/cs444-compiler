#include "tir/Context.h"

#include "tir/Type.h"

namespace tir {

Context::Context(BumpAllocator& alloc) : pimpl_{nullptr} {
   void* buf1 = alloc.allocate_bytes(sizeof(Type), alignof(Type));
   auto* pointerType = new(buf1) Type{};
   void* buf2 = alloc.allocate_bytes(sizeof(Type), alignof(Type));
   auto* voidType = new(buf2) Type{};
   void* buf3 = alloc.allocate_bytes(sizeof(Type), alignof(Type));
   auto* labelType = new(buf3) Type{};
   // Replace pimpl_ with the new ContextPImpl
   void* buf4 = alloc.allocate_bytes(sizeof(ContextPImpl), alignof(ContextPImpl));
   pimpl_ = new(buf4) ContextPImpl{&alloc, pointerType, voidType, labelType};
}

} // namespace tir