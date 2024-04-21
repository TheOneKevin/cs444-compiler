#include "mc/MCPatterns.h"

#include <utility>

#include "mc/InstSelectNode.h"

namespace mc {

using mc::details::MCPatternBase;

std::ostream& MCPatternDef::print(std::ostream& os,
                                  int indent) const DISABLE_ASAN {
   os << std::string(indent * 4, ' ') << getPatternName() << "\n";
   indent++;
   unsigned i = 0;
   for(auto* pat : patterns()) {
      os << std::string(indent * 4, ' ');
      os << "Pattern " << i++ << ":\n";
      pat->print(os, indent + 1);
   }
   return os;
}

void MCPatternDef::dump() const { print(std::cerr); }

InstSelectNode* MCPatterns::matchAndReplace(mc::InstSelectNode* root) const {
   for(auto* def : getPatternFor(root->kind())) {
      for(auto* pat : def->patterns()) {
         if(pat->matches(root)) {
            return root;
         }
      }
   }
   return root;
}

// Print the pattern
std::ostream& MCPatternBase::print(std::ostream& os,
                                   int indent) const DISABLE_ASAN {
   for(auto& bc : bytecode()) {
      os << std::string(indent * 4, ' ');
      switch(bc->type) {
         case MCOperand::Type::Immediate:
            os << "Imm(" << bc->data0 << ")\n";
            break;
         case MCOperand::Type::Register:
            os << "Reg(" << bc->data1 << ")\n";
            break;
         case MCOperand::Type::Push:
            os << "Push\n";
            break;
         case MCOperand::Type::Pop:
            os << "Pop\n";
            break;
         case MCOperand::Type::CheckNodeType:
            os << "CheckNodeType("
               << InstSelectNode::NodeKind_to_string(
                        static_cast<NodeKind>(bc->data1), "??")
               << ")\n";
            break;
         case MCOperand::Type::CheckOperandType:
            os << "CheckOperandType(" << bc->data1 << ")\n";
            break;
         default:
            assert(false);
            std::unreachable();
      }
   }
   return os;
}

void MCPatternBase::dump() const { print(std::cerr); }

bool MCPatternBase::matches(mc::InstSelectNode* node) const {
   for(auto& bc : bytecode()) {
      switch(bc->type) {
         case MCOperand::Type::Push:
         case MCOperand::Type::Pop:
         case MCOperand::Type::CheckNodeType:
         case MCOperand::Type::CheckOperandType:
         default:
            assert(false);
            std::unreachable();
      }
   }
   return false;
}

} // namespace mc
