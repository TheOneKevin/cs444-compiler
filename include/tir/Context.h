#pragma once

#include "utils/BumpAllocator.h"

namespace tir {

class Type;
class FunctionType;
class ArrayType;
class IntegerType;
class StructType;

struct ContextPImpl {
public:
   ContextPImpl(BumpAllocator* alloc, Type* const pointerType,
                Type* const voidType, Type* const labelType)
         : alloc(alloc),
           pointerType(pointerType),
           voidType(voidType),
           labelType(labelType) {}

public:
   BumpAllocator* alloc;
   std::pmr::vector<FunctionType*> functionTypes;
   std::pmr::vector<ArrayType*> arrayTypes;
   std::pmr::vector<IntegerType*> integerTypes;
   std::pmr::vector<StructType*> structTypes;
   Type* const pointerType;
   Type* const voidType;
   Type* const labelType;
};

class Context {
public:
   Context(BumpAllocator& alloc);
   BumpAllocator& alloc() { return *pimpl().alloc; }
   ContextPImpl& pimpl() { return *pimpl_; }

private:
   ContextPImpl* pimpl_;
};

} // namespace tir