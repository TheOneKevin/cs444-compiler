#pragma once

#include <cstdint>
#include <memory>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "diagnostics/Diagnostics.h"
#include "utils/BumpAllocator.h"
#include "third-party/CLI11.h"
#include "utils/Generator.h"
#include <utils/Error.h>

namespace utils {

class PassManager;
class Pass;
class PassOptions;

template <typename T>
concept PassType = std::is_base_of_v<Pass, T>;

class PassOptions {
public:
   PassOptions(CLI::App& app) : app_{app} {}
   PassOptions(PassOptions const&) = delete;
   PassOptions(PassOptions&&) = delete;
   PassOptions& operator=(PassOptions const&) = delete;
   PassOptions& operator=(PassOptions&&) = delete;
   ~PassOptions() = default;

   CLI::Option* FindOption(std::string_view name) {
      std::string test_name = "--" + get_single_name(name);
      if(test_name.size() == 3) test_name.erase(0, 1);
      return app_.get_option_no_throw(test_name);
   }

   CLI::Option* AddOption(std::string name, std::string desc) {
      if(auto opt = FindOption(name)) return opt;
      return app_.add_option(name, CLI::callback_t(), desc);
   }

   CLI::Option* AddFlag(std::string name, std::string desc) {
      if(auto opt = FindOption(name)) return opt;
      return app_.add_flag(
            name, [](int64_t) {}, desc);
   }

   CLI::Option* GetExistingOption(std::string name) {
      auto opt = FindOption(name);
      if(opt == nullptr)
         throw utils::FatalError("pass requested nonexistent option: " + name);
      return opt;
   }

   /// @brief Adds all the pass options to the command line
   void AddAllOptions();

   /// @return True if the pass is disabled
   bool IsPassDisabled(Pass* p);

   /// @brief Enables or disables a pass given the pass name
   void EnablePass(std::string_view name, bool enabled = true) {
      if(auto it = pass_enabled_.find(std::string{name});
         it != pass_enabled_.end()) {
         it->second.enabled = enabled;
      }
   }

private:
   /**
    * @brief Get the single name of a command line option
    * @param name The option name to get the single name of (ie., "-o,--option")
    * @return std::string The single name (ie., "option")
    */
   static std::string get_single_name(std::string_view name) {
      std::vector<std::string> s, l;
      std::string p;
      std::tie(s, l, p) =
            CLI::detail::get_names(CLI::detail::split_names(std::string{name}));
      if(!l.empty()) return l[0];
      if(!s.empty()) return s[0];
      if(!p.empty()) return p;
      return std::string{name};
   }

   /// @brief Sets the pass to be enabled or disabled
   /// @param p The pass to set
   /// @param enabled If true, the pass is enabled
   void setPassEnabled(Pass* p, bool enabled);

   /// @brief Called by PM to parse the command line options
   void parseOptions();

private:
   friend class Pass;
   friend class PassManager;

   CLI::App& app_;
   // A list of passes parsed from the command line
   std::string passes_;
   /// @brief A map of pass names to descriptions and whether they are enabled
   struct PassDesc {
      bool enabled;
      std::string desc;
   };
   std::unordered_map<std::string, PassDesc> pass_enabled_;
};

/* ===--------------------------------------------------------------------=== */
// Pass
/* ===--------------------------------------------------------------------=== */

class Pass {
private:
   // Deleted copy and move constructor and assignment operator
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
   virtual std::string_view Name() const { return ""; }
   /// @brief Function to override to get the description of the pass
   virtual std::string_view Desc() const = 0;

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
   /// @brief Adds a switch to enable the pass
   void RegisterCLI();

private:
   PassManager& pm_;
   enum State {
      Uninitialized,
      PropagateEnabled,
      AcquireResources,
      RegisterDependencies,
      Running,
      Cleanup,
      Valid,
      Invalid
   };
   State state = State::Uninitialized;
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
   PassManager(CLI::App& app)
         : options_{app}, reuseHeaps_{true} {}
   /// @brief Runs all the passes in the pass manager
   /// @return True if all passes ran successfully
   bool Run();
   /// @return The last pass that was run by the pass manager
   Pass const* LastRun() const { return lastRun_; }
   void setHeapReuse(bool reuse) { reuseHeaps_ = reuse; }

private:
   // Deleted copy and move constructor and assignment operator
   PassManager(PassManager const&) = delete;
   PassManager(PassManager&&) = delete;
   PassManager& operator=(PassManager const&) = delete;
   PassManager& operator=(PassManager&&) = delete;

public:
   ~PassManager() {
      // Clean up passes, and then heaps
      passes_.clear();
      heaps_.clear();
   }

public:
   /// @brief Adds a pass to the pass manager
   /// @tparam T The type of the passr
   /// @param ...args The remaining arguments to pass to the pass constructor.
   /// The pass manager will be passed to the pass as the first argument.
   template <typename T, typename... Args>
      requires PassType<T>
   T& AddPass(Args&&... args) {
      passes_.emplace_back(new T(*this, std::forward<Args>(args)...));
      T& result = *cast<T*>(passes_.back().get());
      // If the pass has a name, register it as constructible from the command
      // line options
      if(!result.Name().empty()) result.RegisterCLI();
      return result;
   }
   /// @brief Gets a reference to the diagnostic engine
   diagnostics::DiagnosticEngine& Diag() { return diag_; }
   /// @brief Gets the pass options
   PassOptions& PO() { return options_; }

private:
   template <typename T>
      requires PassType<T>
   T& getPass(Pass& pass) {
      T* result = nullptr;
      for(auto& pass : passes_) {
         if(auto* p = dyn_cast<T*>(pass.get())) {
            if(result != nullptr)
               throw utils::FatalError("Multiple passes of type: " +
                                        std::string(typeid(T).name()));
            result = p;
         }
      }
      if(result == nullptr) {
         throw utils::FatalError("Pass not found: " +
                                  std::string(typeid(T).name()));
      }
      // If the requester is running, the result must be valid
      if(pass.state == Pass::Running && result->state != Pass::Valid) {
         throw utils::FatalError("Pass not valid: " +
                                  std::string(typeid(T).name()));
      }
      return *result;
   }

   template <typename T>
      requires PassType<T>
   Generator<T*> getPasses(Pass& pass) {
      bool found = false;
      for(auto& pass : passes_) {
         if(auto* p = dyn_cast<T*>(pass.get())) {
            // If the requester is running, the result must be valid
            if(p->state == Pass::Running && p->state != Pass::Valid) {
               throw utils::FatalError("Pass not valid: " +
                                        std::string(typeid(T).name()));
            }
            co_yield p;
            found = true;
         }
      }
      if(!found)
         throw utils::FatalError("Pass of type not found: " +
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
   PassOptions options_;
   bool reuseHeaps_;
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
