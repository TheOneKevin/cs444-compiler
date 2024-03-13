#include "semantic/DataflowAnalysis.h"

#include <iostream>

namespace semantic {

using DFA = DataflowAnalysis;
void DFA::LiveVariableAnalysis() const {
   for(auto cu : lu->compliationUnits()) {
      if(auto classDecl = dyn_cast_or_null<ast::ClassDecl>(cu->body())) {
         for(auto method : classDecl->methods()) {
            if(method->body()) {
               auto cfg = cfgBuilder->build(method->body());
               cfg->printDot(std::cout);
            }
         }
         for(auto method : classDecl->constructors()) {
            if(method->body()) {
               auto cfg = cfgBuilder->build(method->body());
               cfg->printDot(std::cout);
            }
         }
      }
   }
}
} // namespace semantic