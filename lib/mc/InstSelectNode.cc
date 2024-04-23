#include "mc/InstSelectNode.h"
#include "tir/Constant.h"
#include "utils/DotPrinter.h"

namespace mc {

using ISN = InstSelectNode;

bool ISN::operator==(InstSelectNode const& other) const {
   switch(kind_) {
      case mc::NodeKind::FrameIndex:
         return get<StackSlot>().idx == other.get<StackSlot>().idx;
      case mc::NodeKind::Register:
      case mc::NodeKind::Argument:
         return get<VReg>().idx == other.get<VReg>().idx;
      default:
         return this == &other;
   }
}

ISN* ISN::selectPattern(BumpAllocator& alloc,
                        details::MCPatternDefBase const* pattern,
                        std::vector<InstSelectNode*>& operands,
                        std::vector<InstSelectNode*>& nodesToDelete) {
   // Create a new node
   void* buf = alloc.allocate(sizeof(ISN));
   auto* newNode = new(buf) ISN{alloc,
                                NodeKind::MachineInstr,
                                static_cast<unsigned>(operands.size()),
                                pattern,
                                Type{0}};
   // Copy the operands
   for(auto* op : operands) {
      newNode->addChild(op);
   }
   // Copy the chains of each nodesToDelete
   for(auto* node : nodesToDelete) {
      for(auto* chain : node->chains()) {
         newNode->addChild(chain);
      }
   }
   // RAUW the root node
   nodesToDelete[0]->replaceAllUsesWith(newNode);
   // Delete each node in nodesToDelete
   for(auto* node : nodesToDelete) {
      node->destroy();
   }
   // Return the new node
   return newNode;
}

void ISN::printNodeTable(utils::DotPrinter& dp) const {
   auto colspan = arity_ + (arity_ < numChildren() ? 1 : 0);
   auto type = type_;
   dp.print_html_start("tr");
   {
      dp.print_html_start("td", {"colspan", std::to_string(colspan)});
      dp.sanitize(NodeKind_to_string(kind_, "??"));
      dp.print_html_end("td");
   }
   dp.print_html_end("tr");
   dp.print_html_start("tr");
   {
      dp.print_html_start("td", {"colspan", std::to_string(colspan)});
      if(type.bits == 0) {
         dp.sanitize("Type: void");
      } else {
         dp.sanitize("Type: i" + std::to_string(type.bits));
      }
      dp.print_html_end("td");
   }
   dp.print_html_end("tr");
   dp.print_html_start("tr");
   {
      for(unsigned i = 0; i < arity_; i++) {
         auto str = std::to_string(i);
         dp.print_html_start("td", {"port", str});
         dp.sanitize(str);
         dp.print_html_end("td");
      }
      if(arity_ < numChildren()) {
         dp.print_html_start("td", {"port", "ch"});
         dp.sanitize("ch");
         dp.print_html_end("td");
      }

      dp.print_html_start("td", {"port", "topoIdx"});
      dp.sanitize(std::to_string(topoIdx()));
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
   int id = (kind() == NodeKind::Entry) ? dp.getId(this) : dp.id(this);
   // If it's a leaf node, print the data. Otherwise, just print the operation
   switch(kind()) {
      case NodeKind::Constant: {
         auto imm = get<ImmValue>();
         dp.startTLabel(id, {"style", "filled", "bgcolor", "gainboro"});
         dp.printTableSingleRow("Constant");
         dp.printTableDoubleRow("Value", std::to_string(imm.value));
         dp.printTableDoubleRow("Bits", std::to_string(imm.bits));
         dp.endTLabel();
         break;
      }
      case NodeKind::Register: {
         auto reg = get<VReg>().idx;
         dp.startTLabel(id, {"style", "filled", "bgcolor", "lightblue"});
         dp.printTableSingleRow("Register");
         dp.printTableDoubleRow("VReg", std::to_string(reg));
         dp.endTLabel();
         break;
      }
      case NodeKind::FrameIndex: {
         auto idx = get<StackSlot>().idx;
         dp.startTLabel(id, {"style", "filled", "bgcolor", "lightblue"});
         dp.printTableSingleRow("FrameIndex");
         dp.printTableDoubleRow("Idx", std::to_string(idx));
         dp.endTLabel();
         break;
      }
      case NodeKind::Argument: {
         auto idx = get<VReg>().idx;
         dp.startTLabel(id, {"style", "filled", "bgcolor", "lightblue"});
         dp.printTableSingleRow("Argument");
         dp.printTableDoubleRow("Idx", std::to_string(idx));
         dp.endTLabel();
         break;
      }
      case NodeKind::BasicBlock: {
         // Defer the printing of connections to MCFunction
         dp.printLabel(
               id, "BasicBlock", {"style", "filled", "fillcolor", "lightblue"});
         return id;
      }
      case NodeKind::Entry: {
         dp.printLabel(id,
                       "Entry",
                       {"style", "filled", "fillcolor", "lightgreen"},
                       "Mdiamond");
         break;
      }
      case NodeKind::GlobalAddress: {
         auto GO = get<tir::GlobalObject*>();
         dp.startTLabel(id, {"style", "filled", "bgcolor", "lightblue"});
         dp.printTableSingleRow("GlobalAddress");
         dp.printTableDoubleRow("Id", GO->name());
         dp.endTLabel();
         break;
      }
      default: {
         if(arity_ == 0) {
            dp.printLabel(id,
                          NodeKind_to_string(kind_, "??"),
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
   for(i = 0; i < (unsigned)arity_; i++) {
      auto child = cast<ISN>(getRawChild(i));
      int childId = child->printDotNode(dp, visited);
      dp.indent() << "node" << id << ":" << i << ":s -> node" << childId
                  << "[weight=100];\n";
   }
   // For the rest, print it as red
   for(; i < numChildren(); i++) {
      auto child = cast<ISN>(getRawChild(i));
      int childId = child->printDotNode(dp, visited);
      dp.indent() << "node" << id << ":ch:s -> node" << childId
                  << " [color=red weight=0];\n";
   }
   return id;
}

void ISN::buildAdjacencyList(std::unordered_map<ISN*, std::vector<ISN*>> &adj) {
   for(auto* child : users()) {
      if (child->kind() == NodeKind::Constant || child->kind() == NodeKind::Register 
      || child->kind() == NodeKind::FrameIndex || child->kind() == NodeKind::Argument 
      || child->kind() == NodeKind::BasicBlock || child->kind() == NodeKind::Entry 
      || child->kind() == NodeKind::GlobalAddress) {
         continue;
      }

      adj[this].push_back(cast<ISN>(child));
   }
   
   for(unsigned i = numChildren(); i > 0; i--) {
      auto child = cast<ISN>(getRawChild(i - 1));

      if (child->kind() == NodeKind::Constant || child->kind() == NodeKind::Register 
      || child->kind() == NodeKind::FrameIndex || child->kind() == NodeKind::Argument 
      || child->kind() == NodeKind::BasicBlock || child->kind() == NodeKind::Entry 
      || child->kind() == NodeKind::GlobalAddress) {
         continue;
      }

      child->buildAdjacencyList(adj);
   }
}

utils::Generator<ISN*> ISN::childNodes() const {
   for(auto* child : children()) {
      co_yield cast<ISN>(child);
   }
}

} // namespace mc