#include "mc/MCPatterns.h"

#include <stack>
#include <utility>

#include "mc/InstSelectNode.h"
#include "mc/MCTargetDesc.h"

namespace mc {

using mc::details::MCPatternBase;

std::ostream& MCPatternDef::print(std::ostream& os, int indent) const {
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

// Print the pattern
std::ostream& MCPatternBase::print(std::ostream& os, int indent) const {
   for(auto& bc : bytecode()) {
      os << std::string(indent * 4, ' ');
      switch(bc->type) {
         case MCOperand::Type::Immediate:
            os << "Imm(" << bc->data << ")\n";
            break;
         case MCOperand::Type::Register:
            os << "Reg(" << bc->data << ")\n";
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
                        static_cast<NodeKind>(bc->data), "??")
               << ")\n";
            break;
         case MCOperand::Type::CheckOperandType:
            os << "CheckOperandType(" << bc->data << ")\n";
            break;
         default:
            assert(false);
            std::unreachable();
      }
   }
   return os;
}

void MCPatternBase::dump() const { print(std::cerr); }

bool MCPatternBase::matches(MatchOptions opt) const {
   auto [TD, def, ops, nodesToDelete, node] = opt;
   using N = mc::NodeKind;
   using I = mc::InstSelectNode;
   std::stack<std::pair<int, mc::InstSelectNode*>> stack;
   stack.push({0, node});
   unsigned childIdx = 0;
   nodesToDelete.push_back(node);
   for(auto& bc : bytecode()) {
      if(childIdx >= node->arity()) return false;
      switch(bc->type) {
         case MCOperand::Type::Push: {
            node = node->getChild(childIdx);
            stack.push({childIdx, node});
            childIdx++;
            nodesToDelete.push_back(node);
            break;
         }
         case MCOperand::Type::Pop: {
            stack.pop();
            childIdx = stack.top().first;
            node = stack.top().second;
            break;
         }
         case MCOperand::Type::CheckNodeType: {
            auto expected = static_cast<N>(bc->data);
            if(node->kind() != expected) return false;
            break;
         }
         case MCOperand::Type::CheckOperandType: {
            auto index = bc->data;
            auto operand = def->getInput(index);
            auto child = node->getChild(childIdx++);
            // Check if the operand matches what we've seen before
            if(!ops[index]) {
               ops[index] = child;
            } else if(*ops[index] != *child) {
               return false;
            }
            // Check if the operand matches the expected type
            switch(operand.type) {
               case MCOperand::Type::Immediate: {
                  if(child->kind() != N::Constant ||
                     child->get<I::ImmValue>().bits != operand.data)
                     return false;
                  break;
               }
               case MCOperand::Type::Register: {
                  if(!TD.isRegisterClass(operand.data, child->type()))
                     return false;
                  break;
               }
               default:
                  return false;
                  assert(false);
                  std::unreachable();
            }
            break;
         }
         default:
            assert(false);
            std::unreachable();
      }
   }
   return true;
}

} // namespace mc
