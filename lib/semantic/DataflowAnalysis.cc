#include "semantic/DataflowAnalysis.h"

#include <algorithm>
#include <iostream>
#include <unordered_map>
#include "diagnostics/Location.h"

namespace semantic {

using DFA = DataflowAnalysis;
void DFA::Check() const {
   for(auto cu : lu->compliationUnits()) {
      if(auto classDecl = dyn_cast_or_null<ast::ClassDecl>(cu->body())) {
         for(auto method : classDecl->methods()) {
            if(method->body()) {
               auto cfg = cfgBuilder->build(method->body());
               cfg->printDot(std::cout);
               ReachabilityCheck(cfg);

               if (method->returnTy().type) {
                  FiniteLengthReturn(cfg);
               }
            }
         }
         for(auto method : classDecl->constructors()) {
            if(method->body()) {
               auto cfg = cfgBuilder->build(method->body());
               // cfg->printDot(std::cout);
               ReachabilityCheck(cfg);
            }
         }
      }
   }
}

void DFA::FiniteLengthReturn(const CFGNode* node) const {
   if (node->hasBeenVisited()) {
      return;
   }

   node->setVisited(true);

   if (node->isReturnNode()) {
      return;
   }
   
   if (node->getChildren().begin() == node->getChildren().end()) {
      if (std::holds_alternative<CFGNode::EmptyExpr>(node->getData())) {
         diag.ReportError(SourceRange {}) << " Missing return statement here.";
      } else {
         diag.ReportError(node->location().value()) << " Missing return statement here.";
      }
      
      return;
   }
   
   for (auto child : node->getChildren()) {
      FiniteLengthReturn(child);
   }
}

void DFA::ReachabilityCheck(const CFGNode* node) const {
   std::unordered_map<const CFGNode*, bool> out;
   std::pmr::set<const CFGNode*> worklist{alloc};
   getAllNodes(node, worklist);
   for(auto n : worklist) {
      out[n] = true;
   }
   std::pmr::set<const CFGNode*> next{alloc};
   ReachabilityCheckHelper(out, worklist, next);
   for(auto n : worklist) {
      bool reachable = n == node;
      for(auto parent : n->getParents()) {
         if(out[parent]) {
            reachable = true;
            break;
         }
      }
      if(!reachable) {
         if (n->location()) {
            diag.ReportError(n->location().value()) << "Unreachable statement";
         } else {
            diag.ReportError(node->location().value()) << "Unreachable statement";
         }
      }
   }
}

void DFA::ReachabilityCheckHelper(std::unordered_map<const CFGNode*, bool>& out,
                                  std::pmr::set<const CFGNode*>& cur,
                                  std::pmr::set<const CFGNode*>& next) const {
   if(cur.empty()) return;
   for(auto n : cur) {
      bool newOut = false;
      if(n->isStart() && !n->isReturnNode()) {
         newOut = true;
      } else if(!n->isReturnNode()) {
         for(auto child : n->getParents()) {
            if(out[child]) { // if it is reachable from a parent, it is reachable
               newOut = true;
               break;
            }
         }
      }
      if(newOut != out[n]) {
         out[n] = newOut;
         for(auto child : n->getChildren()) {
            next.insert(child);
         }
      }
   }
   std::pmr::set<const CFGNode*> next_next{alloc};
   ReachabilityCheckHelper(out, next, next_next);
}

void DFA::getAllNodes(const CFGNode* node,
                      std::pmr::set<const CFGNode*>& list) const {
   if(list.count(node) > 0) {
      return;
   }
   list.insert(node);
   for(auto child : node->getChildren()) {
      getAllNodes(child, list);
   }
}

} // namespace semantic