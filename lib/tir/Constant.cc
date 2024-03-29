#include "tir/Constant.h"

#include <iostream>

#include "tir/BasicBlock.h"
#include "tir/Type.h"
#include "utils/DotPrinter.h"

namespace tir {

ConstantInt* Constant::CreateInt(Context& ctx, uint8_t bits, uint32_t value) {
   return ConstantInt::Create(ctx, IntegerType::get(ctx, bits), value);
}

ConstantNullPointer* Constant::CreateNullPointer(Context& ctx) {
   return ConstantNullPointer::Create(ctx);
}

std::ostream& ConstantNullPointer::print(std::ostream& os) const {
   os << "ptr* null";
   return os;
}

std::ostream& ConstantInt::print(std::ostream& os) const {
   os << *type() << " " << zextValue();
   return os;
}

std::ostream& GlobalVariable::print(std::ostream& os) const { return os; }

Argument::Argument(Function* parent, Type* type, unsigned index)
      : Value{parent->ctx(), type}, parent_{parent}, index_{index} {
   setName("arg");
}

std::ostream& Argument::print(std::ostream& os) const {
   os << *type() << " ";
   printName(os);
   return os;
}

std::ostream& Function::print(std::ostream& os) const {
   os << "function ";
   if(isExternalLinkage()) os << "external ";
   if(isNoReturn()) os << "noreturn ";
   os << *getReturnType() << " "
      << "@" << name() << "(";
   bool isFirst = true;
   for(auto* arg : args()) {
      if(!isFirst) os << ", ";
      isFirst = false;
      arg->print(os);
   }
   os << ")";
   if(hasBody()) {
      os << " {\n";
      for(auto* bb : body()) {
         bb->print(os) << "\n";
      }
      os << "}\n";
   }
   return os;
}

void Function::printDot(std::ostream& os) const {
   utils::DotPrinter dp{os, "5"};
   dp.startGraph();
   std::unordered_map<BasicBlock*, int> bbMap;
   std::vector<BasicBlock*> terms;
   for(auto bb : body()) {
      int id = bb->printDotNode(dp);
      bbMap[bb] = id;
   }
   for(auto bb : body()) {
      terms.clear();
      for(auto succ : bb->successors()) terms.push_back(succ);
      if(terms.size() == 2) {
         dp.printConnection(bbMap[bb], ":T", bbMap[terms[0]]);
         dp.printConnection(bbMap[bb], ":F", bbMap[terms[1]]);
      } else {
         for (auto term : terms) {
            dp.printConnection(bbMap[bb], bbMap[term]);
         }
      }
   }
   dp.endGraph();
}

} // namespace tir
