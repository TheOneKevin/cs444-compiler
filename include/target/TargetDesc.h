#pragma once

#include "mc/InstSelectNode.h"
#include "mc/Patterns.h"

namespace target {

enum class ArchType;

/**
 * @brief Abstract class that describes the MC target
 */
class TargetDesc {
public:
   /// @brief Returns the pattern provider for DAG pattern matching
   virtual const mc::PatternProviderBase& patternProvider() const = 0;
   /**
    * @brief Checks if the register given class can be used for the given
    * instruction selection node type
    *
    * @param classIdx The class index
    * @param type The instruction selection node type
    */
   virtual bool isRegisterClass(unsigned classIdx,
                                mc::InstSelectNode::Type type) const = 0;

public:
   /**
    * @brief
    *
    * @tparam ArchType
    * @return TargetDesc&
    */
   template <ArchType>
   static TargetDesc& Get();
};

} // namespace target