#include "mc/InstSelectNode.h"

namespace mc {

using ISN = InstSelectNode;

int ISN::printDotNode(utils::DotPrinter& dp,
                      std::unordered_set<ISN const*>& visited) const {
   // If we have visited this node, return the id
   if(visited.count(this)) return dp.getId(this);
   visited.insert(this);
   // Create a new the id for this node
   int id = (type() == NodeType::Entry) ? dp.getId(this) : dp.id(this);
   // If it's a leaf node, print the data. Otherwise, just print the operation
   switch(type()) {
      case NodeType::Constant: {
         auto imm = std::get<ImmValue>(data_.value());
         dp.startTLabel(id, {"style", "filled", "bgcolor", "gainboro"});
         dp.printTableSingleRow("Constant");
         dp.printTableDoubleRow("Value", std::to_string(imm.value));
         dp.printTableDoubleRow("Bits", std::to_string(imm.bits));
         dp.endTLabel();
         break;
      }
      case NodeType::Register: {
         auto reg = std::get<VReg>(data_.value()).idx;
         dp.startTLabel(id, {"style", "filled", "bgcolor", "lightblue"});
         dp.printTableSingleRow("Register");
         dp.printTableDoubleRow("VReg", std::to_string(reg));
         dp.endTLabel();
         break;
      }
      case NodeType::FrameIndex: {
         auto idx = std::get<StackSlot>(data_.value()).idx;
         dp.startTLabel(id, {"style", "filled", "bgcolor", "lightblue"});
         dp.printTableSingleRow("FrameIndex");
         dp.printTableDoubleRow("Idx", std::to_string(idx));
         dp.endTLabel();
         break;
      }
      case NodeType::Argument: {
         auto idx = std::get<VReg>(data_.value()).idx;
         dp.startTLabel(id, {"style", "filled", "bgcolor", "lightblue"});
         dp.printTableSingleRow("Argument");
         dp.printTableDoubleRow("Idx", std::to_string(idx));
         dp.endTLabel();
         break;
      }
      case NodeType::BasicBlock: {
         // Defer the printing of connections to MCFunction
         dp.printLabel(
               id, "BasicBlock", {"style", "filled", "fillcolor", "lightblue"});
         return id;
      }
      case NodeType::Entry: {
         dp.printLabel(id,
                       "Entry",
                       {"style", "filled", "fillcolor", "lightgreen"},
                       "Mdiamond");
         break;
      }
      default:
         if(arity == 0) {
            dp.printLabel(id,
                          NodeType_to_string(type_, "??"),
                          {"style", "filled", "fillcolor", "lightblue"});
         } else {
            dp.printLabel(id, NodeType_to_string(type_, "??"));
         }
         break;
   }
   // Now go to the children
   unsigned i;
   for(i = 0; i < (unsigned)arity; i++) {
      auto child = cast<ISN>(getRawChild(i));
      int childId = child->printDotNode(dp, visited);
      dp.printConnection(id, childId);
   }
   // For the rest, print it as red
   for(; i < numChildren(); i++) {
      auto child = cast<ISN>(getRawChild(i));
      int childId = child->printDotNode(dp, visited);
      dp.printConnection(id, childId, {"color", "red"});
   }
   return id;
}

utils::Generator<ISN*> ISN::childNodes() const {
   for(auto* child : children()) {
      co_yield cast<ISN>(child);
   }
}

} // namespace mc