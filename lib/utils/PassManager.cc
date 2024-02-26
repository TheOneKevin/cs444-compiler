#include "utils/PassManager.h"

namespace utils {

/* ===--------------------------------------------------------------------=== */
// Pass
/* ===--------------------------------------------------------------------=== */

void Pass::ComputeDependency(Pass& pass) {
   // Request any non-temporary heaps from the pass
   for(auto& heap : PM().heaps_) {
      if(heap.owner == &pass) {
         if(!running_) {
            heap.refCount++;
         } else {
            PM().freeHeap(heap);
         }
      }
   }
   if(!running_) {
      PM().addDependency(*this, pass);
   }
}

CustomBufferResource* Pass::NewHeap() { return PM().newHeap(*this); }

/* ===--------------------------------------------------------------------=== */
// PassManager
/* ===--------------------------------------------------------------------=== */

CustomBufferResource* PassManager::newHeap(Pass& pass) {
   // First, find a free heap
   for(auto& heap : heaps_) {
      if(heap.refCount != 0) continue;
      // We found a free heap, so we can use it
      heap.refCount = 1;
      heap.owner = &pass;
      return heap.heap.get();
   }
   // If we can't find a free heap, then create a new one
   heaps_.emplace_back(&pass);
   return heaps_.back().heap.get();
}

void PassManager::freeHeap(Heap& h) {
   // If fully is true, then we destroy the heap
   // Otherwise, just decrement the ref count
   if(h.refCount > 0) {
      h.refCount--;
   }
   // If the ref count is 0, then we can destroy the heap
   if(h.refCount == 0) {
      h.owner = nullptr;
      h.heap->reset();
   }
   return;
}

bool PassManager::Run() {
   std::vector<Pass*> S;
   std::vector<Pass*> L;
   // First, calculate the order of the passes
   for(auto& pass : passes_) {
      passDeps_[pass.get()] = 0;
      // Compute dependencies will also request any heaps
      pass->computeDependencies();
      if(passDeps_[pass.get()] == 0) S.push_back(pass.get());
   }
   // Then, run the topological sort
   while(!S.empty()) {
      auto* n = *S.begin();
      S.erase(S.begin());
      L.push_back(n);
      for(auto* m : depgraph_[n]) {
         if(--passDeps_[m] == 0) S.push_back(m);
      }
      depgraph_[n].clear();
   }
   // Check for cycles
   if(L.size() != passes_.size())
      throw std::runtime_error("Cyclic pass dependency detected");
   // Then, run the passes in order
   for(auto& pass : L) {
      assert(!pass->valid_ && "Pass already run");
      pass->running_ = true;
      if(Diag().Verbose()) {
         Diag().ReportDebug() << "[=>] Running " << pass->Name() << " Pass";
      }
      pass->Run();
      lastRun_ = pass;
      if(Diag().hasErrors()) {
         pass->running_ = false;
         pass->valid_ = false;
         return false;
      }
      // If the pass is running, we can free the heaps by calling
      // computeDependencies again
      pass->computeDependencies();
      // Update the pass state
      pass->running_ = false;
      pass->valid_ = true;
   }
   return true;
}

} // namespace utils
