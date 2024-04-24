#include "mc/MCPatterns.h"

#include <stack>
#include <utility>

#include "mc/InstSelectNode.h"
#include "target/Target.h"
#include "target/TargetDesc.h"

namespace mc {

std::ostream& MCPatternDef::print(std::ostream& os, int indent) const {
   os << std::string(indent * 4, ' ') << name() << "\n";
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
std::ostream& MCPattern::print(std::ostream& os, int indent) const {
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

void MCPattern::dump() const { print(std::cerr); }

unsigned MCPatternDef::adjustOperandIndex(unsigned index,
                                          target::TargetDesc const& TD) const {
   using mc::details::MCOperand;
   unsigned counter = 0;
   assert(index <= numInputs() && "Index out of bounds");
   for(unsigned i = 0; i < index; i++) {
      auto curop = getInput(i);
      if(curop.type == MCOperand::Type::Fragment) {
         // FIXME(kevin): Assumes fragments do not contain fragments
         auto& Frag = TD.getMCPatterns().getFragment(curop.data);
         counter += std::max<unsigned>(1, Frag.numInputs());
      } else {
         counter++;
      }
   }
   return counter;
}

bool MCPattern::matches(MatchOptions opt) const {
   auto [TD, def, ops, nodesToDelete, node] = opt;
   using N = mc::NodeKind;
   using I = mc::InstSelectNode;
   std::stack<std::pair<int, mc::InstSelectNode*>> stack;
   stack.push({0, node});
   unsigned childIdx = 0;
   nodesToDelete.push_back(node);
   // Check if the output types matches the pattern output
   assert(def->numOutputs() <= 1 && "Multiple outputs not supported");
   if(def->numOutputs() == 0) {
      if(node->type().bits != 0) return false;
   } else if(def->numOutputs() == 1) {
      if(def->getOutput(0).type == MCOperand::Type::Register) {
         if(!TD.isRegisterClass(def->getOutput(0).data, node->type()))
            return false;
      }
   }
   // Run the bytecode logic
   for(auto& bc : bytecode()) {
      if((bc->type != MCOperand::Type::Pop) && (childIdx >= node->arity()))
         return false;
      switch(bc->type) {
         case MCOperand::Type::Push: {
            node = node->getChild(childIdx);
            stack.push({childIdx, node});
            childIdx = 0;
            nodesToDelete.push_back(node);
            break;
         }
         case MCOperand::Type::Pop: {
            stack.pop();
            childIdx = stack.top().first + 1;
            node = stack.top().second;
            break;
         }
         case MCOperand::Type::CheckNodeType: {
            auto expected = static_cast<N>(bc->data);
            if(node->kind() != expected) return false;
            break;
         }
         case MCOperand::Type::CheckOperandType: {
            const auto index = bc->data;
            const auto operand = def->getInput(index);
            const auto child = node->getChild(childIdx++);
            const auto adjustedIndex = def->adjustOperandIndex(index, TD);
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
               case MCOperand::Type::Fragment: {
                  InstSelectNode* newChild = nullptr;
                  auto optCopy = opt;
                  optCopy.node = child;
                  auto& P = TD.getMCPatterns();
                  auto& Frag = P.getFragment(operand.data);
                  if(!P.matchFragment(Frag, optCopy, newChild)) return false;
                  if(newChild) ops[adjustedIndex] = newChild;
                  break;
               }
               default:
                  assert(false);
                  std::unreachable();
            }
            // Check if the operand matches what we've seen before
            if(!ops[adjustedIndex]) {
               ops[adjustedIndex] = child;
            } else if(*ops[adjustedIndex] != *child) {
               return false;
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
