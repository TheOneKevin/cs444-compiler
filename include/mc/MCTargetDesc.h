#pragma once

#include <cstdint>
namespace mc {

/// @brief Describes a target register
struct MCRegDesc final {
   int32_t index;      ///< The index of the register in getMCRegDesc(index)
   uint32_t classMask; ///< The register class mask
};

/// @brief Abstract class that describes the MC target
class MCTargetDesc {
public:
   /// @brief Gets the number of MC register classes
   virtual int numMCRegClasses() const = 0;
   /// @brief Gets the total number of distinct MC registers
   virtual int numMCRegisters() const = 0;
   /// @brief Gets the MC register description for the i-th MC register
   /// where i is in the range [0, numMCRegisters())
   virtual MCRegDesc getMCRegDesc(int i) const = 0;
};

} // namespace mc
