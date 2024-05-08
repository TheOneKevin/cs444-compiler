#pragma once

#include <memory>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "diagnostics/Diagnostics.h"
#include "third-party/CLI11.h"
#include "utils/BumpAllocator.h"
#include "utils/Error.h"
#include "utils/Generator.h"
#include "utils/Utils.h"

namespace utils {

class PassManager;
class Pass;
class PassDispatcher;
class PassOptions;

template <typename T>
concept PassType = std::is_base_of_v<Pass, T>;

template <typename T>
concept DispatchType = std::is_base_of_v<PassDispatcher, T>;

/* ===--------------------------------------------------------------------=== */
// Pass
/* ===--------------------------------------------------------------------=== */

class Pass {
public:
   Pass(Pass const&) = delete;
   Pass(Pass&&) = delete;
   Pass& operator=(Pass const&) = delete;
   Pass& operator=(Pass&&) = delete;

public:
   virtual ~Pass() = default;
   /// @brief Function to override when you want to acquire resources
   virtual void Init() {}
   /// @brief Function to override to run the pass
   virtual void Run() = 0;
   /// @brief Function to override to get the name (id) of the pass
   virtual std::string_view Name() const = 0;
   /// @brief Function to override to get the description of the pass
   virtual std::string_view Desc() const = 0;
   /// @brief The tag for this pass
   virtual int Tag() const { return 0; }
   /// @brief Preserve the analysis results of this pass
   void Preserve() { preserve = true; }
   /// @brief Should this pass be preserved?
   bool ShouldPreserve() const { return preserve; }
   /// @brief Garbage collect any persistent resources. This is a FIXME(kevin)!
   virtual void GC() {};

public:
   enum class Lifetime { Managed, Temporary, TemporaryNoReuse };

protected:
   /// @brief Gets the pass manager that owns the pass
   auto& PM() { return pm_; }

   /// @brief Gets a single pass of type T. Throws if no pass is found.
   /// Also throws if multiple passes of type T are found.
   /// @tparam T The type of the pass
   /// @return T& The pass of type T
   template <PassType T>
   T& GetPass();

   /// @brief Gets a single pass by name. Throws if no pass is found.
   Pass& GetPass(std::string_view name);

   /// @brief Gets all passes of type T. Throws if no pass is found.
   /// @tparam T The type of the pass
   /// @return A generator that yields all passes of type T
   template <PassType T>
   Generator<T*> GetPasses();

   /// @brief Computes a dependency between this and another pass
   /// @param pass The pass to add as a dependency
   void AddDependency(Pass& pass);

   /**
    * @brief Obtains a new heap resource given the lifetime
    *
    * @param lifetime Dictates the lifetime of the heap resource
    * @return CustomBufferResource* The heap resource
    */
   CustomBufferResource* NewHeap(Lifetime lifetime);

   /**
    * @brief Obtains a new bump allocator given the lifetime
    *
    * @param lifetime The lifetime of the bump allocator
    * @return BumpAllocator& The bump allocator
    */
   BumpAllocator& NewAlloc(Lifetime lifetime);

   /// @brief Gets the dispatcher of type T
   template <DispatchType T>
   T* GetDispatcher() {
      return cast<T>(dispatcher);
   }

protected:
   friend class PassManager;
   /// @brief Overload to state the dependencies of this pass
   virtual void ComputeDependencies() = 0;
   /// @brief Constructor for the pass
   /// @param pm The pass manager that owns the pass
   explicit Pass(PassManager& pm) noexcept : pm_(pm) {}

private:
   PassManager& pm_;
   enum class State {
      Uninitialized /* Before and during Init() */,
      Initialized /* After Init() */,
      Running /* During Run() */,
      Valid /* After Run() or before managed resources are freed */,
      Invalid /* After Run() and at least 1 managed resource is freed */
   };
   State state = State::Uninitialized;
   bool preserve = false;
   bool enabled = false;
   int topoIdx = -1;
   PassDispatcher* dispatcher = nullptr;
   std::vector<BumpAllocator> allocs_;
};

/* ===--------------------------------------------------------------------=== */
// PassDispatcher
/* ===--------------------------------------------------------------------=== */

class PassDispatcher {
public:
   virtual ~PassDispatcher() = default;
   virtual std::string_view Name() = 0;
   virtual bool CanDispatch(Pass& pass) = 0;
   virtual Generator<void*> Iterate(PassManager&) = 0;
};

/* ===--------------------------------------------------------------------=== */
// PassManager
/* ===--------------------------------------------------------------------=== */

class PassManager final {
   friend class Pass;
   struct Chunk {
      int left, right;
      PassDispatcher* dispatcher;
   };
   struct HeapResource;

public:
   PassManager(CLI::App& app) : app_{app}, reuseHeaps_{true} {}
   PassManager(PassManager const&) = delete;
   PassManager(PassManager&&) = delete;
   PassManager& operator=(PassManager const&) = delete;
   PassManager& operator=(PassManager&&) = delete;
   ~PassManager();

public:
   /// @brief Initializes the pass manager
   void Init();
   /// @brief Runs all the passes in the pass manager
   /// @return True if all passes ran successfully
   bool Run();
   /// @brief Resets the pass manager and frees all resources
   void Reset();
   /// @return The last pass that was run by the pass manager
   Pass const* LastRun() const { return lastRun_; }
   /// @brief Sets whether the pass manager should reuse heaps
   void SetHeapReuse(bool reuse) { reuseHeaps_ = reuse; }
   /// @brief Adds a pass to the pass manager
   /// @tparam T The type of the pass
   /// @param ...args The remaining arguments to pass to the pass constructor.
   /// The pass manager will be passed to the pass as the first argument.
   template <PassType T, typename... Args>
   T& AddPass(Args&&... args);
   /// @brief Adds a dispatcher to the pass manager
   /// @tparam T The type of the dispatcher
   /// @param ...args The remaining arguments to pass to the dispatcher
   /// constructor. The pass manager will be passed to the dispatcher as the
   /// first argument.
   template <DispatchType T, typename... Args>
   void AddDispatcher(Args&&... args);
   /// @brief Gets a reference to the diagnostic engine
   diagnostics::DiagnosticEngine& Diag() { return diag_; }
   /// @brief External method to get a pass by type. Can be used before the
   /// pass pipeline is run to set individual pass's properties.
   template <PassType T>
   T& FindPass();
   /// @brief Gets a single pass by name. Throws if no pass is found.
   Pass& FindPass(std::string_view name) {
      for(auto& pass : passes_)
         if(!pass->Name().empty() && pass->Name() == name) return *pass;
      throw FatalError("Pass not found: " + std::string{name});
   }
   /// @brief
   CLI::Option* FindOption(std::string_view name);
   /// @brief
   CLI::Option* GetExistingOption(std::string name);
   /// @return True if the pass is disabled
   bool IsPassDisabled(Pass& p) { return p.enabled; }
   /// @brief Enables or disables a pass given the pass name
   void EnablePass(std::string_view name, bool enabled = true) {
      FindPass(name).enabled = enabled;
   }
   /// @brief Iterate through the pass names
   Generator<Pass const*> Passes() {
      for(auto& pass : passes_) co_yield pass.get();
   }
   /// @returns True if the pass manager has a pass with the given name
   bool HasPass(std::string_view name) {
      for(auto& pass : passes_)
         if(!pass->Name().empty() && pass->Name() == name) return true;
      return false;
   }

private:
   // Internal function used by Pass to find a pass
   template <PassType T>
   T& getPass(Pass& pass);
   // Shared function used by getPass and FindPass
   template <PassType T>
   T& getPass(bool checkInit);
   // Internal function to get all passes of a type
   template <PassType T>
   Generator<T*> getPasses(Pass& pass);

private:
   void runPassLifeCycle(Pass& pass, int left, int right, bool lastIter);
   void addDependency(Pass& pass, Pass& depends);
   void validate() const;
   HeapResource& findHeapFor(Pass* pass, Pass::Lifetime);

private:
   enum class State {
      Uninitialized /* Before or during Init() */,
      Initialized /* After Init() or after Reset() */,
      Running /* During Run() */,
      Cleanup /* After Run() or during Reset() */
   };

   struct GraphEdge /* Of vertex u */ {
      std::vector<Pass*> forward;   // Stores u->children
      std::vector<Pass*> transpose; // Stores predecessor->u
   };

   struct HeapResource final {
      using T = CustomBufferResource;
      int id = counter++;
      Pass* owner = nullptr;
      Pass::Lifetime lifetime = Pass::Lifetime::Managed;
      unsigned refcount = 0;
      std::unique_ptr<T> resource{nullptr};
      HeapResource(std::unique_ptr<T>&& resource)
            : resource{std::move(resource)} {}
      T* get() { return resource.get(); }

   private:
      static inline int counter = 0;
   };

private:
   CLI::App& app_;
   std::vector<std::unique_ptr<Pass>> passes_;
   std::vector<Chunk> passChunks_;
   std::vector<std::unique_ptr<PassDispatcher>> dispatchers_;
   std::vector<HeapResource> heaps_;
   diagnostics::DiagnosticEngine diag_;
   Pass* lastRun_ = nullptr;
   bool reuseHeaps_;
   State state_ = State::Uninitialized;
   std::unordered_map<Pass*, GraphEdge> depGraph_;
};

/* ===--------------------------------------------------------------------=== */
// Template implementations
/* ===--------------------------------------------------------------------=== */

template <PassType T>
T& Pass::GetPass() {
   return PM().getPass<T>(*this);
}

template <PassType T>
Generator<T*> Pass::GetPasses() {
   return PM().getPasses<T>(*this);
}

template <PassType T>
T& PassManager::FindPass() {
   return getPass<T>(true);
}

template <PassType T, typename... Args>
T& PassManager::AddPass(Args&&... args) {
   using namespace std;
   passes_.emplace_back(make_unique<T>(*this, std::forward<Args>(args)...));
   T& result = *cast<T*>(passes_.back().get());
   return result;
}

template <DispatchType T, typename... Args>
void PassManager::AddDispatcher(Args&&... args) {
   using namespace std;
   dispatchers_.emplace_back(make_unique<T>(std::forward<Args>(args)...));
}

template <PassType T>
T& PassManager::getPass(bool checkInit) {
   T* result = nullptr;
   for(auto& pass : passes_) {
      if(auto* p = dyn_cast<T*>(pass.get())) {
         if(result != nullptr)
            throw FatalError("Multiple passes of type: " +
                             std::string(typeid(T).name()));
         result = p;
      }
   }
   if(result == nullptr) {
      throw FatalError("Pass not found: " + std::string(typeid(T).name()));
   }
   // FIXME(kevin): This check needs to be fixed
   if(checkInit && result->state == Pass::State::Running) {
      throw FatalError("Cannot use FindPass() while a pass is running");
   }
   return *result;
}

template <PassType T>
T& PassManager::getPass(Pass& pass) {
   T& result = getPass<T>(false);
   // If the requester is running, the result must be valid
   if(pass.state == Pass::State::Running && result.state != Pass::State::Valid) {
      throw FatalError("Pass not valid: " + std::string(typeid(T).name()));
   }
   return result;
}

template <PassType T>
Generator<T*> PassManager::getPasses(Pass&) {
   bool found = false;
   for(auto& pass : passes_) {
      if(auto* p = dyn_cast<T*>(pass.get())) {
         // If the requester is running, the result must be valid
         if(p->state == Pass::State::Running && p->state != Pass::State::Valid) {
            throw FatalError("Pass not valid: " + std::string(typeid(T).name()));
         }
         co_yield p;
         found = true;
      }
   }
   if(!found)
      throw FatalError("Pass of type not found: " + std::string(typeid(T).name()));
}

/* ===--------------------------------------------------------------------=== */
// Macro definitions to export
/* ===--------------------------------------------------------------------=== */

/**
 * @brief Registers NS::T with the pass manager.
 */
#define REGISTER_PASS_NS(NS, T)                        \
   utils::Pass& New##T##Pass(utils::PassManager& PM) { \
      return PM.AddPass<NS::T>();                      \
   }

/**
 * @brief Registers T with the pass manager.
 */
#define REGISTER_PASS(T) \
   utils::Pass& New##T##Pass(utils::PassManager& PM) { return PM.AddPass<T>(); }

/**
 * @brief Forward declares a "new pass" function.
 */
#define DECLARE_PASS(T) utils::Pass& New##T##Pass(utils::PassManager& PM);

} // namespace utils
