#pragma once

#include "mc/InstSelectNode.h"
#include "mc/MCPatterns.h"

namespace mc {

struct MCRegDesc {};

/// @brief Abstract class that describes the MC target
class MCTargetDesc {
public:
   /// @brief Initializes any target-specific information
   virtual void initialize() = 0;
   /// @brief Gets the number of MC register classes
   virtual int numMCRegClasses() const = 0;
   /// @brief Gets the total number of distinct MC registers
   virtual int numMCRegisters() const = 0;
   /// @brief Returns the MCPattern class for DAG pattern matching
   virtual const MCPatterns& getMCPatterns() const = 0;
   /**
    * @brief Checks if the register given class can be used for the given
    * instruction selection node type
    *
    * @param classIdx The class index
    * @param type The instruction selection node type
    */
   virtual bool isRegisterClass(unsigned classIdx,
                                InstSelectNode::Type type) const = 0;
};

} // namespace mc
