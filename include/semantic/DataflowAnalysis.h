#pragma once
#include "ast/DeclContext.h"
#include "semantic/CFGBuilder.h"
namespace semantic {

class DataflowAnalysis {
   using Heap = std::pmr::memory_resource;

public:
   DataflowAnalysis(diagnostics::DiagnosticEngine& diag, Heap* heap, ast::Semantic& sema, 
                    ast::LinkingUnit* lu)
         : diag{diag}, alloc{heap}, heap{heap}, sema{sema}, lu{lu} {}
   void init(CFGBuilder* cfgBuilder) { this->cfgBuilder = cfgBuilder; }
   void LiveVariableAnalysis() const;

private:
   diagnostics::DiagnosticEngine& diag;
   CFGBuilder* cfgBuilder;
   mutable BumpAllocator alloc;
   Heap* heap;
   ast::Semantic& sema;
   ast::LinkingUnit* lu;
};

} // namespace semantic
