#include "mc/InstSelectNode.h"

#include "mc/MCFunction.h"
#include "mc/MCPatterns.h"
#include "tir/Constant.h"
#include "utils/DotPrinter.h"

namespace mc {

using ISN = InstSelectNode;

static std::string typeToString(ISN::Type type) {
   if(type.bits == 0) {
      return "void";
   } else {
      return "i" + std::to_string(type.bits);
   }
}

bool ISN::operator==(InstSelectNode const& other) const {
   if(kind_ != other.kind_) return false;
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

ISN* ISN::selectPattern(MatchOptions options) {
   auto& alloc = parent_->alloc();
   auto [TD, pattern, operands, nodesToDelete, node] = options;
   // Create a new node
   void* buf = alloc.allocate(sizeof(ISN));
   auto* newNode = new(buf) ISN{alloc,
                                NodeKind::MachineInstr,
                                static_cast<unsigned>(operands.size()),
                                pattern,
                                Type{0},
                                parent_};
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
   this->replaceAllUsesWith(newNode);
   // Delete each node in nodesToDelete
   // FIXME(kevin): We really don't need this part of it
   /*for(auto* node : nodesToDelete) {
      node->destroy();
   }*/
   // Return the new node
   return newNode;
}

void ISN::printNodeTable(utils::DotPrinter& dp) const {
   auto colspan = arity_ + (arity_ < numChildren() ? 1 : 0);
   auto type = type_;
   // Row: Node kind name
   dp.print_html_start("tr");
   {
      dp.print_html_start("td", {"colspan", std::to_string(colspan)});
      dp.sanitize(NodeKind_to_string(kind_, "??"));
      dp.print_html_end("td");
   }
   dp.print_html_end("tr");
   // Row: Type or MCInstr name
   dp.print_html_start("tr");
   {
      dp.print_html_start("td", {"colspan", std::to_string(colspan)});
      if(kind_ == NodeKind::MachineInstr) {
         auto* pat = get<details::MCPatDefBase const*>();
         dp.sanitize(pat->name());
      } else {
         dp.sanitize("Type: " + typeToString(type));
      }
      dp.print_html_end("td");
   }
   dp.print_html_end("tr");
   // Row: TopoIdx
   if(topoIdx_ >= 0) {
      dp.print_html_start("tr");
      dp.print_html_start("td", {"colspan", std::to_string(colspan)});
      dp.sanitize("TopoIdx: " + std::to_string(topoIdx()));
      dp.print_html_end("td");
      dp.print_html_end("tr");
   }
   // Row: Children ports
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
         dp.startTLabel(id, {"style", "filled"}, "1", "gainsboro");
         dp.printTableSingleRow("Constant");
         dp.printTableDoubleRow("Value", std::to_string(imm.value));
         dp.printTableDoubleRow("Bits", std::to_string(imm.bits));
         dp.endTLabel();
         break;
      }
      case NodeKind::Register: {
         auto reg = get<VReg>().idx;
         dp.startTLabel(id, {"style", "filled"}, "1", "gainsboro");
         dp.printTableSingleRow("Register");
         dp.printTableDoubleRow("VReg", std::to_string(reg));
         dp.endTLabel();
         break;
      }
      case NodeKind::FrameIndex: {
         auto idx = get<StackSlot>().idx;
         dp.startTLabel(id, {"style", "filled"}, "1", "gainsboro");
         dp.printTableSingleRow("FrameIndex");
         dp.printTableSingleRow("Type: " + typeToString(type_));
         dp.printTableDoubleRow("Idx", std::to_string(idx));
         dp.endTLabel();
         break;
      }
      case NodeKind::Argument: {
         auto idx = get<VReg>().idx;
         dp.startTLabel(id, {"style", "filled"}, "1", "gainsboro");
         dp.printTableSingleRow("Argument");
         dp.printTableDoubleRow("Idx", std::to_string(idx));
         dp.endTLabel();
         break;
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
         dp.startTLabel(id, {"style", "filled"}, "1", "gainsboro");
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
            if(kind_ == NodeKind::BasicBlock) return id;
         } else {
            dp.startTLabel(
                  id,
                  {},
                  "3",
                  (kind_ == NodeKind::MachineInstr) ? "lightpink" : "white");
            printNodeTable(dp);
            dp.endTLabel();
         }
         break;
      }
   }
   // Now go to the children
   unsigned i;
   for(i = 0; i < (unsigned)arity_; i++) {
      if(!getRawChild(i)) continue;
      auto child = cast<ISN>(getRawChild(i));
      int childId = child->printDotNode(dp, visited);
      dp.indent() << "node" << id << ":" << i << ":s -> node" << childId
                  << "[weight=100];\n";
   }
   // For the rest, print it as red
   for(; i < numChildren(); i++) {
      if(!getRawChild(i)) continue;
      auto child = cast<ISN>(getRawChild(i));
      int childId = child->printDotNode(dp, visited);
      dp.indent() << "node" << id << ":ch:s -> node" << childId
                  << " [color=red weight=0];\n";
   }
   return id;
}

utils::Generator<ISN*> ISN::childNodes() const {
   for(auto* child : children()) {
      co_yield child ? cast<ISN>(child) : nullptr;
   }
}

} // namespace mc