#pragma once

#include <memory_resource>
#include <unordered_map>

#include "tir/Constant.h"
#include "tir/Context.h"

namespace tir {

class CompilationUnit {
public:
   CompilationUnit(Context& ctx) : ctx_{ctx}, functions_{ctx.alloc()} {}

   /**
    * @brief Create a new function with the given type and name.
    *
    * @param type The type of the function
    * @param name The name of the function
    * @return Function* The function, or nullptr if it already exists
    */
   Function* CreateFunction(FunctionType* type, const std::string_view name) {
      if(functions_.find(std::string{name}) != functions_.end()) return nullptr;
      auto* buf = ctx_.alloc().allocate_bytes(sizeof(Function), alignof(Function));
      auto* func = new(buf) Function{ctx_, this, type, name};
      functions_.emplace(name, func);
      return func;
   }

   /**
    * @brief Get the function with the given name.
    *
    * @param name The name of the function
    * @return Function* The function, or nullptr if it does not exist
    */
   Function* GetFunction(const std::string_view name) {
      auto it = functions_.find(std::string{name});
      if(it == functions_.end()) {
         return nullptr;
      }
      return it->second;
   }

private:
   Context& ctx_;
   std::pmr::unordered_map<std::string, Function*> functions_;
};

} // namespace tir
