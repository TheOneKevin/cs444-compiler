#include "mc/InstSelectNode.h"

#include "utils/DotPrinter.h"

namespace mc {

using ISN = InstSelectNode;

void ISN::printNodeTable(utils::DotPrinter& dp) const {
   auto colspan = arity + (arity < numChildren() ? 1 : 0);
   dp.print_html_start("tr");
   dp.print_html_start("td", {"colspan", std::to_string(colspan)});
   dp.sanitize(NodeType_to_string(type_, "??"));
   dp.print_html_end("td");
   dp.print_html_end("tr");
   dp.print_html_start("tr");
   for(unsigned i = 0; i < arity; i++) {
      auto str = std::to_string(i);
      dp.print_html_start("td", {"port", str});
      dp.sanitize(str);
      dp.print_html_end("td");
   }
   if(arity < numChildren()) {
      dp.print_html_start("td", {"port", "ch"});
      dp.sanitize("ch");
      dp.print_html_end("td");
   }
   dp.print_html_end("tr");
}

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
      default: {
         if(arity == 0) {
            dp.printLabel(id,
                          NodeType_to_string(type_, "??"),
                          {"style", "filled", "fillcolor", "lightblue"});
         } else {
            dp.startTLabel(id, {}, "3");
            printNodeTable(dp);
            dp.endTLabel();
         }
         break;
      }
   }
   // Now go to the children
   unsigned i;
   for(i = 0; i < (unsigned)arity; i++) {
      auto child = cast<ISN>(getRawChild(i));
      int childId = child->printDotNode(dp, visited);
      dp.indent() << "node" << id << ":" << i << ":s -> node" << childId << "[weight=100];\n";
   }
   // For the rest, print it as red
   for(; i < numChildren(); i++) {
      auto child = cast<ISN>(getRawChild(i));
      int childId = child->printDotNode(dp, visited);
      dp.indent() << "node" << id << ":ch:s -> node" << childId << " [color=red weight=0];\n";
   }
   return id;
}

utils::Generator<ISN*> ISN::childNodes() const {
   for(auto* child : children()) {
      co_yield cast<ISN>(child);
   }
}

} // namespace mc