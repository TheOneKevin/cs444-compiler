#include "Target.h"

// Global target descriptor
static target::x86::x86TargetDesc TD;

// Global target info
static target::x86::x86TargetInfo TI;

namespace target::x86 {

// Checks if the given class can take an MIR type
bool x86TargetDesc::isRegisterClass(unsigned classIdx,
                                    mc::InstSelectNode::Type type) const {
   if(classIdx == (unsigned)x86RegClass::GPR32) {
      return type.bits == 32;
   } else if(classIdx == (unsigned)x86RegClass::GPR64) {
      return type.bits == 64;
   }
   return false;
}

} // namespace target::x86

template <>
target::TargetDesc& target::TargetDesc::Get<target::ArchType::X86>() {
   return TD;
}

template <>
target::TargetInfo& target::TargetInfo::Get<target::ArchType::X86>() {
   return TI;
}
