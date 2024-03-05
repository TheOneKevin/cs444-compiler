#include <third-party/CLI11.h>
#include <utils/PassManager.h>

#include <sstream>
#include <utils/Error.h>

namespace utils {

/* ===--------------------------------------------------------------------=== */
// PassOptions
/* ===--------------------------------------------------------------------=== */

bool PassOptions::IsPassDisabled(Pass* p) {
   auto it = pass_enabled_.find(std::string{p->Name()});
   if(it == pass_enabled_.end()) return false;
   return !it->second.enabled;
}

void PassOptions::setPassEnabled(Pass* p, bool enabled) {
   pass_enabled_[std::string{p->Name()}].enabled = enabled;
}

void PassOptions::parseOptions() {
   // Iterate over the string, split by commas
   std::string_view str = passes_;
   while(!str.empty()) {
      auto pos = str.find(',');
      std::string_view pass = str.substr(0, pos);
      if(auto it = pass_enabled_.find(std::string{pass});
         it != pass_enabled_.end()) {
         it->second.enabled = true;
      }
      if(pos == std::string_view::npos) break;
      str.remove_prefix(pos + 1);
   }
}

void PassOptions::AddAllOptions() {
   std::ostringstream oss;
   oss << "A comma separated list of passes to run. The list of passes is: \n";
   // Find the longest name
   size_t max_len = 0;
   for(auto& [key, _] : pass_enabled_) max_len = std::max(max_len, key.size());
   // Now add the list of passes
   for(auto& [key, val] : pass_enabled_) {
      oss << "  " << key;
      for(size_t i = 0; i < max_len - key.size(); i++) oss << " ";
      oss << " : " << val.desc << "\n";
   }
   app_.add_option("-p,--passes", passes_, oss.str());
}

/* ===--------------------------------------------------------------------=== */
// Pass
/* ===--------------------------------------------------------------------=== */

void Pass::ComputeDependency(Pass& pass) {
   // This function serves a many purposes depending on the state
   // 1. PropagateEnabled: Recursively propagate the enabled state
   // 2. AcquireResources: We acquire any heap resources
   // 3. RegisterDependencies: Register the dep with PM to build the depgraph
   // 4. Cleanup: We only free the heap resources

   if(state == PropagateEnabled) {
      // If we're disabled, then we don't need to do anything
      if(PM().PO().IsPassDisabled(this)) return;
      // Otherwise, we propagate the enabled state
      PM().PO().setPassEnabled(&pass, true);
      pass.state = PropagateEnabled;
      pass.computeDependencies();
      return;
   }

   for(auto& heap : PM().heaps_) {
      if(heap.owner != &pass) continue;
      if(state == AcquireResources) {
         heap.refCount++;
      } else if(state == Cleanup) {
         PM().freeHeap(heap);
      }
   }

   if(state == RegisterDependencies) {
      PM().addDependency(*this, pass);
   }
}

CustomBufferResource* Pass::NewHeap() { return PM().newHeap(*this); }

void Pass::RegisterCLI() {
   auto& PO = PM().PO();
   PO.pass_enabled_[std::string{Name()}] =
         PassOptions::PassDesc{false, std::string{Desc()}};
}

Pass& Pass::GetPass(std::string_view name) {
   PM().getPass(name);
}

/* ===--------------------------------------------------------------------=== */
// PassManager
/* ===--------------------------------------------------------------------=== */

CustomBufferResource* PassManager::newHeap(Pass& pass) {
   // Check the pass is running
   if(pass.state != Pass::Running && pass.state != Pass::AcquireResources) {
      throw utils::FatalError("Pass requesting a heap is not running");
   }
   // First, find a free heap
   for(auto& heap : heaps_) {
      if(heap.owner != nullptr) continue;
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
      if(Diag().Verbose(2)) {
         Diag().ReportDebug() << "[=>] Freeing heap for " << h.owner->Desc();
      }
      if(reuseHeaps_) {
         h.owner = nullptr;
         h.heap->reset();
      } else {
         h.heap->destroy();
      }
   }
   return;
}

bool PassManager::Run() {
   std::vector<Pass*> S;
   std::vector<Pass*> L;
   PO().parseOptions();
   // 1a Propagate the enabled state
   for(auto& pass : passes_) {
      pass->state = Pass::PropagateEnabled;
      pass->computeDependencies();
   }
   // 1b Acquire resources for the passes
   for(auto& pass : passes_) {
      // If the pass is disabled, then we skip it
      if(PO().IsPassDisabled(pass.get())) continue;
      pass->state = Pass::AcquireResources;
      pass->Init();
   }
   // 1c Propagate the heap refcounts
   for(auto& pass : passes_) {
      if(PO().IsPassDisabled(pass.get())) continue;
      pass->computeDependencies();
   }
   if(Diag().Verbose(2)) {
      for(auto& heap : heaps_) {
         Diag().ReportDebug(2) << "[=>] Heap Owner: \"" << heap.owner->Desc()
                               << "\" RefCount: " << heap.refCount;
      }
   }
   // 2. Build the dependency graph of passes
   size_t NumEnabledPasses = 0;
   for(auto& pass : passes_) {
      if(PO().IsPassDisabled(pass.get())) continue;
      NumEnabledPasses++;
      passDeps_[pass.get()] = 0;
      pass->state = Pass::RegisterDependencies;
      pass->computeDependencies();
      // Mark the pass in the graph
      if(passDeps_[pass.get()] == 0) S.push_back(pass.get());
   }
   // 3. Run topological sort on the dependency graph
   while(!S.empty()) {
      auto* n = *S.begin();
      S.erase(S.begin());
      L.push_back(n);
      for(auto* m : depgraph_[n]) {
         if(--passDeps_[m] == 0) S.push_back(m);
      }
      depgraph_[n].clear();
   }
   // 4. Check for cycles
   if(L.size() != NumEnabledPasses)
      throw utils::FatalError("Cyclic pass dependency detected");
   // 5. Run the passes in topological order
   for(auto& pass : L) {
      assert(pass->state != Pass::Valid && "Pass already run");
      pass->state = Pass::Running;
      if(Diag().Verbose()) {
         Diag().ReportDebug() << "[=>] Running " << pass->Desc() << " Pass";
      }
      pass->Run();
      lastRun_ = pass;
      if(Diag().hasErrors()) {
         pass->state = Pass::Invalid;
         return false;
      }
      // If the pass is running, we can free the heaps by calling
      // computeDependencies again
      pass->state = Pass::Cleanup;
      pass->computeDependencies();
      // Update the pass state
      pass->state = Pass::Valid;
   }
   return true;
}

} // namespace utils
