#pragma once

#include <iostream>
#include <ranges>
#include <set>
#include <string>
#include <vector>

#include "parsetree/ParseTree.h"
#include "utils/DotPrinter.h"
#include "utils/Generator.h"

namespace semantic {
class NameResolver;
} // namespace semantic

namespace ast {

using utils::DotPrinter;

template <typename T>
using pmr_vector = std::pmr::vector<T>;
template <typename T>
using array_ref = std::pmr::vector<T>&;
using std::string_view;

// Base class for all AST nodes ////////////////////////////////////////////////

class Type;
class BuiltInType;
class Decl;
class DeclContext;
class Stmt;

class Expr;

/// @brief Base class for all AST nodes. Helps unify printing and dot printing.
class AstNode {
   friend class Expr;

public:
   AstNode() = default;
   AstNode(const AstNode&) = delete;
   AstNode(AstNode&&) = delete;
   AstNode& operator=(const AstNode&) = delete;
   AstNode& operator=(AstNode&&) = delete;

public:
   std::ostream& printDot(std::ostream& os) const {
      DotPrinter dp{os};
      dp.startGraph();
      dp.print("compound=true;");
      printDotNode(dp);
      dp.endGraph();
      return os;
   }

   virtual std::ostream& print(std::ostream& os, int indentation = 0) const = 0;
   virtual ~AstNode() = default;
   virtual int printDotNode(DotPrinter& dp) const = 0;
   void dump() const;
   /// @brief Returns a generator for the children of this node.
   virtual utils::Generator<AstNode const*> children() const = 0;
   /// @brief Returns a generator for the mutable children of this node.
   utils::Generator<AstNode*> mut_children() {
      for(auto const* child : children()) co_yield const_cast<AstNode*>(child);
   }

protected:
   /**
    * @brief Get a string of spaces for indentation
    *
    * @param indentation The level of indentation
    * @return std::string String of spaces
    */
   static std::string indent(int indentation) {
      return std::string(indentation * 2, ' ');
   }
};

/// @brief Base class for all declarations.
class Decl : public AstNode {
public:
   Decl(BumpAllocator& alloc, std::string_view name) noexcept
         : canonicalName_{alloc}, name_{name, alloc}, parent_{nullptr} {}

   /// @brief Gets the simple name of this declaration.
   std::string_view name() const { return name_; }
   /// @brief Gets the context in which this declaration is declared.
   DeclContext* parent() const { return parent_; }
   /// @brief Sets the parent. See parent().
   virtual void setParent(DeclContext* parent) {
      assert(parent_ == nullptr);
      parent_ = parent;
   }
   /// @brief Gets the fully qualified name of this declaration. Returns
   /// undefined value if the declaration does not have a canonical name.
   std::string_view getCanonicalName() const {
      assert(hasCanonicalName() && "Does not have a canonical name.");
      assert(parent_ != nullptr && "Canonical name requires a non-null parent.");
      return canonicalName_;
   }
   /// @brief Returns if the declaration has a canonical name.
   virtual bool hasCanonicalName() const = 0;
   /// @brief Returns the location of the declaration. This is an abstract
   /// method to allow abstract classes of Decl without location.
   virtual SourceRange location() const = 0;

protected:
   std::pmr::string canonicalName_;

private:
   std::pmr::string name_;
   DeclContext* parent_;
};

/// @brief Base class for all declaration contexts (i.e., methods).
class DeclContext : public AstNode {
public:
};

/// @brief Base class for all types.
class Type : public AstNode {
public:
   Type(SourceRange loc) : loc_{loc} {}
   virtual string_view toString() const = 0;
   std::ostream& print(std::ostream& os, int indentation = 0) const override {
      return os << indent(indentation) << toString();
   }
   int printDotNode(DotPrinter& dp) const override {
      int id = dp.id();
      dp.printLabel(id, toString());
      return id;
   }
   SourceRange location() const { return loc_; }
   /// @brief Resolves the type based on the condition of isResolved()
   virtual void resolve(semantic::NameResolver&) {}
   /// @brief Returns if the type is resolved
   virtual bool isResolved() const = 0;
   virtual bool operator==(const Type&) const = 0;
   virtual bool operator!=(const Type& other) { return !(*this == other); }
   /// @brief Since there is no child, this returns an empty generator.
   /// Note: We don't count the cross-reference to the declaration as a child
   ///       as that would violate the "tree" part of AST.
   utils::Generator<AstNode const*> children() const override final {
      co_yield nullptr;
   }

private:
   SourceRange loc_;
};

/// @brief Base class for all statements.
class Stmt : public AstNode {
public:
   /// @brief By default, returns an empty generator for the statement.
   virtual utils::Generator<AstNode const*> children() const override {
      co_yield nullptr;
   }
};

/// @brief Overload the << operator for AstNode to print the node
std::ostream& operator<<(std::ostream& os, const AstNode& astNode);

/**
 * @brief Prints the dot node for each item in the range. The connections
 * are then formed as first -> second -> third -> fourth -> ...
 * and the ID of the first node is returned.
 *
 * @tparam Range This is inferred
 * @param dp The DotPrinter
 * @param range The range must be an iterable of ast::AstNode*
 * @return The ID of the first node
 */
template <std::ranges::range Range>
   requires std::is_convertible_v<std::ranges::range_value_t<Range>, ast::AstNode*>
int printDotNodeList(DotPrinter& dp, Range&& range) {
   int childIdFirst = -1;
   int childIdLast = -1;
   for(auto p : range) {
      int childId = p->printDotNode(dp);
      if(childIdLast != -1)
         dp.printConnection(childIdLast, childId);
      else
         childIdFirst = childId;
      childIdLast = childId;
   }
   return childIdFirst;
}

/**
 * @brief Draws either a single statement node or a subgraph of statements
 * if the statement is a block statement. Returns the ID of the first node
 * and the ID of the subgraph if it is a block statement.
 *
 * @param dp The DotPrinter
 * @param stmt The statement to draw
 * @return std::pair<int, int> Returns -1 if stmt is not a block statement
 */
std::pair<int, int> printStmtSubgraph(utils::DotPrinter& dp, ast::Stmt* stmt);

// Other classes ///////////////////////////////////////////////////////////////

class Modifiers {
public:
   enum class Type {
      Public = 0,
      Protected = 1,
      Static = 2,
      Final = 3,
      Abstract = 4,
      Native = 5,
      NumModifiers = 6
   };

public:
   /// @brief Will clear + set the modifier given a parsetree::Modifier
   /// Will also set the location of the modifier.
   /// @param modifier The modifier to assign to this Modifiers object
   void set(parsetree::Modifier target);

   /// @brief Will union the modifier with the current modifiers
   /// @param modifier The modifier to union with this Modifiers object
   /// @return True if the modifier was already set
   bool set(ast::Modifiers::Type target) {
      bool wasSet = test(modifiers, target);
      modifiers |= (1 << (uint8_t)target);
      return wasSet;
   }

   /// @brief Will union the modifiers with the current modifiers
   /// @param target The set of modifiers to union
   /// @return True if any of the modifiers were already set
   bool set(ast::Modifiers target) {
      bool wasSet = false;
      for(int i = 0; i < (int)Type::NumModifiers; i++) {
         if(test(target.modifiers, (Type)i)) {
            wasSet |= set((Type)i);
         }
      }
      return wasSet;
   }

   /// @brief Returns an iterator over the locations of the modifiers that
   /// are set in both this Modifiers object and the given Modifiers object.
   /// @param target The set of modifiers to intersect
   auto getLocationsMasked(Modifiers target) const {
      auto masked = target.modifiers & modifiers;
      return std::views::iota(0, (int)Type::NumModifiers) |
             std::views::filter(
                   [masked](int i) { return (masked & (1 << i)) != 0; }) |
             std::views::transform([this](int i) { return modifierLocations[i]; });
   }

   /// @brief Returns the location of the given modifier. Returns an
   /// undefined location if the modifier is not set.
   auto getLocation(Type modifier) const {
      return modifierLocations[(int)modifier];
   }

   bool isPublic() const { return test(modifiers, Type::Public); }
   bool isProtected() const { return test(modifiers, Type::Protected); }
   bool isStatic() const { return test(modifiers, Type::Static); }
   bool isFinal() const { return test(modifiers, Type::Final); }
   bool isAbstract() const { return test(modifiers, Type::Abstract); }
   bool isNative() const { return test(modifiers, Type::Native); }

   std::string toString() const;

private:
   SourceRange modifierLocations[(int)Type::NumModifiers];
   uint8_t modifiers = 0;

   static constexpr int test(uint8_t value, Type bit) {
      return (value & (1 << (uint8_t)bit)) != 0;
   }
};

} // namespace ast
