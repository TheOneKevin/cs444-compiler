#pragma once

#include <memory>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "diagnostics/Diagnostics.h"
#include "utils/BumpAllocator.h"
#include "utils/Generator.h"

namespace utils {

class PassManager;
class Pass;

template <typename T>
concept PassType = std::is_base_of_v<Pass, T>;

/* ===--------------------------------------------------------------------=== */
// Pass
/* ===--------------------------------------------------------------------=== */

class Pass {
public:
   // Deleted copy and move constructor and assignment operator
   Pass(Pass const&) = delete;
   Pass(Pass&&) = delete;
   Pass& operator=(Pass const&) = delete;
   Pass& operator=(Pass&&) = delete;

public:
   virtual ~Pass() = default;

   /// @brief Function to override to run the pass
   virtual void Run() = 0;

   /// @brief Function to override to get the name of the pass
   virtual std::string_view Name() const = 0;

protected:
   /// @brief Gets the pass manager that owns the pass
   auto& PM() { return pm_; }

   /// @brief Gets a single pass of type T. Throws if no pass is found.
   /// Also throws if multiple passes of type T are found.
   /// @tparam T The type of the pass
   /// @return T& The pass of type T
   template <typename T>
      requires PassType<T>
   T& GetPass();

   /// @brief Gets all passes of type T. Throws if no pass is found.
   /// @tparam T The type of the pass
   /// @return A generator that yields all passes of type T
   template <typename T>
      requires PassType<T>
   Generator<T*> GetPasses();

protected:
   /// @brief Computes a dependency between this and another pass
   /// @param pass The pass to add as a dependency
   void ComputeDependency(Pass& pass);
   /// @brief Requests a new heap from the pass manager
   /// @return CustomBufferResource* The new heap
   CustomBufferResource* NewHeap();

protected:
   friend class PassManager;
   /// @brief Overload to state the dependencies of this pass
   virtual void computeDependencies() = 0;
   /// @brief Constructor for the pass
   /// @param pm The pass manager that owns the pass
   explicit Pass(PassManager& pm) noexcept : pm_(pm) {}

private:
   PassManager& pm_;
   bool valid_ = false;
   bool running_ = false;
};

/* ===--------------------------------------------------------------------=== */
// PassManager
/* ===--------------------------------------------------------------------=== */

class PassManager final {
private:
   /// @brief A heap is a bump allocator that is used to allocate memory. The
   /// heap is owned by a pass and is destroyed when no future passes will
   /// require it.
   struct Heap {
      std::unique_ptr<CustomBufferResource> heap;
      Pass* owner;
      int refCount;
      Heap(Pass* owner)
            : heap{std::make_unique<CustomBufferResource>()},
              owner{owner},
              refCount{1} {}
   };

public:
   bool Run();
   Pass const* LastRun() const { return lastRun_; }

public:
   /// @brief Adds a pass to the pass manager
   /// @tparam T The type of the passr
   /// @param ...args The remaining arguments to pass to the pass constructor.
   /// The pass manager will be passed to the pass as the first argument.
   template <typename T, typename... Args>
   // requires PassType<T>
   T& AddPass(Args&&... args) {
      passes_.emplace_back(new T(*this, std::forward<Args>(args)...));
      return *dynamic_cast<T*>(passes_.back().get());
   }

   /// @brief Gets a reference to the diagnostic engineF
   diagnostics::DiagnosticEngine& Diag() { return diag_; }

private:
   template <typename T>
      requires PassType<T>
   T& getPass(Pass& pass) {
      T* result = nullptr;
      for(auto& pass : passes_) {
         if(auto* p = dynamic_cast<T*>(pass.get())) {
            if(result != nullptr)
               throw std::runtime_error("Multiple passes of type: " +
                                        std::string(typeid(T).name()));
            result = p;
         }
      }
      if(result == nullptr) {
         throw std::runtime_error("Pass not found: " +
                                  std::string(typeid(T).name()));
      }
      // If the requester is running, the result must be valid
      if(pass.running_ && !result->valid_) {
         throw std::runtime_error("Pass not valid: " +
                                  std::string(typeid(T).name()));
      }
      return *result;
   }

   template <typename T>
      requires PassType<T>
   Generator<T*> getPasses(Pass& pass) {
      bool found = false;
      for(auto& pass : passes_) {
         if(auto* p = dynamic_cast<T*>(pass.get())) {
            // If the requester is running, the result must be valid
            if(p->running_ && !p->valid_) {
               throw std::runtime_error("Pass not valid: " +
                                        std::string(typeid(T).name()));
            }
            co_yield p;
            found = true;
         }
      }
      if(!found)
         throw std::runtime_error("Pass of type not found: " +
                                  std::string(typeid(T).name()));
   }

private:
   friend class Pass;
   friend class HeapRef;
   CustomBufferResource* newHeap(Pass& pass);
   void freeHeap(Heap& heap);
   void addDependency(Pass& pass, Pass& depends) {
      depgraph_[&depends].push_back(&pass);
      passDeps_[&pass]++;
   }

private:
   std::vector<std::unique_ptr<Pass>> passes_;
   std::vector<Heap> heaps_;
   std::unordered_map<Pass*, std::vector<Pass*>> depgraph_;
   diagnostics::DiagnosticEngine diag_;
   Pass* lastRun_ = nullptr;
   std::unordered_map<Pass*, int> passDeps_;
};

template <typename T>
   requires PassType<T>
T& Pass::GetPass() {
   return PM().getPass<T>(*this);
}

template <typename T>
   requires PassType<T>
Generator<T*> Pass::GetPasses() {
   return PM().getPasses<T>(*this);
}

} // namespace utils
