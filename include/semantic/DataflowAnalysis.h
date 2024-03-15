#pragma once
#include "ast/DeclContext.h"
#include "semantic/CFGBuilder.h"
namespace semantic {

class DataflowAnalysis {
   using Heap = std::pmr::memory_resource;

public:
   DataflowAnalysis(diagnostics::DiagnosticEngine& diag, Heap* heap,
                    ast::Semantic& sema, ast::LinkingUnit* lu)
         : diag{diag}, alloc{heap}, heap{heap}, sema{sema}, lu{lu} {}
   void init(CFGBuilder* cfgBuilder) { this->cfgBuilder = cfgBuilder; }
   void Check() const;

private:
   diagnostics::DiagnosticEngine& diag;
   CFGBuilder* cfgBuilder;
   mutable BumpAllocator alloc;
   Heap* heap;
   ast::Semantic& sema;
   ast::LinkingUnit* lu;

   void LiveVariableAnalysis(const CFGNode* node) const;
   void FiniteLengthReturn(const CFGNode* node) const;
   void ReachabilityCheck(const CFGNode* node) const;
   void ReachabilityCheckHelper(std::unordered_map<const CFGNode*, bool>& out,
                                std::pmr::set<const CFGNode*>& cur,
                                std::pmr::set<const CFGNode*>& next) const;
   void getAllNodes(const CFGNode* node,
                    std::pmr::set<const CFGNode*>& list) const;
};

} // namespace semantic
