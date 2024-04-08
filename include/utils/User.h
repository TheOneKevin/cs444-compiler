#pragma once

#include <ranges>
#include <unordered_set>

#include "utils/Assert.h" // IWYU pragma: keep
#include "utils/BumpAllocator.h"
#include "utils/Generator.h"

/* ===--------------------------------------------------------------------=== */
// Use and hashing Use{}
/* ===--------------------------------------------------------------------=== */

namespace utils {

template <typename T>
class GraphNode;

template <typename T>
class GraphNodeUser;

/**
 * @brief Defines a use of a graph node by another graph node. The user
 * of the node is of type T. The fromIndex is the index of the use of the
 * current node in the user's children vector.
 *
 * @tparam T The type of the user class.
 */
template <typename T>
struct Use {
   T* const user;
   const unsigned fromIndex;
};

namespace detail {
template <typename T>
struct UseEqual {
   bool operator()(const Use<T>& lhs, const Use<T>& rhs) const {
      return lhs.user == rhs.user && lhs.fromIndex == rhs.fromIndex;
   }
};
} // namespace detail

} // namespace utils

template <typename T>
struct std::hash<utils::Use<T>> {
   std::size_t operator()(const utils::Use<T>& use) const {
      return std::hash<T*>{}(use.user) ^ std::hash<unsigned>{}(use.fromIndex);
   }
};

/* ===--------------------------------------------------------------------=== */
// GraphNode
/* ===--------------------------------------------------------------------=== */

/**
 * @brief Base class for all graph nodes.
 *
 * @tparam T The type of the user (not graph node) class.
 */
template <typename T>
class utils::GraphNode {
public:
   GraphNode(BumpAllocator& alloc) : uses_{alloc} {}
   virtual ~GraphNode() = default;
   void addUse(Use<T> use) { uses_.insert(use); }
   void removeUse(Use<T> use) { uses_.erase(use); }
   auto uses() const { return std::views::all(uses_); }
   void replaceAllUsesWith(GraphNode* newValue);
   utils::Generator<T*> users() {
      for(auto user : uses_) co_yield static_cast<T*>(user.user);
   }
   utils::Generator<T const*> users() const {
      for(auto use : uses_) co_yield static_cast<T*>(use.user);
   }

protected:
   std::pmr::unordered_set<Use<T>, std::hash<Use<T>>, detail::UseEqual<T>> uses_;
};

/* ===--------------------------------------------------------------------=== */
// GraphNodeUser
/* ===--------------------------------------------------------------------=== */

/**
 * @brief Base class for graph nodes that have children (i.e., they "use"
 * other nodes). You must also separately inherit from GraphNode.
 *
 * @tparam T The type of the user class.
 */
template <typename T>
class utils::GraphNodeUser {
   friend class GraphNode<T>;
   using Node = GraphNode<T>;

public:
   GraphNodeUser(BumpAllocator& alloc) : children_{alloc} {}
   auto children() const { return std::views::all(children_); }
   auto numChildren() const { return children_.size(); }
   Node* getRawChild(unsigned idx) const {
      assert(idx < numChildren() && "Index out of bounds");
      return children_[idx];
   }

protected:
   void addChild(Node* operand) {
      children_.push_back(operand);
      operand->addUse({static_cast<T*>(this), (unsigned)numChildren() - 1});
   }
   void addChild(Node* operand, unsigned idx) {
      children_.insert(children_.begin() + idx, operand);
      operand->addUse({static_cast<T*>(this), idx});
   }
   void replaceChild(unsigned idx, Node* operand) {
      assert(idx < numChildren() && "Index out of bounds");
      children_[idx]->removeUse({static_cast<T*>(this), idx});
      children_[idx] = operand;
      operand->addUse({static_cast<T*>(this), idx});
   }
   void destroy() {
      assert(!destroyed_);
      unsigned idx = 0;
      for(auto child : children_) {
         child->removeUse({static_cast<T*>(this), idx++});
      }
   }
   bool isDestroyed() const { return destroyed_; }

protected:
   std::pmr::vector<Node*> children_;
   bool destroyed_ = false;
};

template <typename T>
void utils::GraphNode<T>::replaceAllUsesWith(GraphNode<T>* newValue) {
   for(auto use : uses_) {
      use.user->replaceChild(use.fromIndex, newValue);
   }
}
