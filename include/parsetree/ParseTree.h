#pragma once

#include <array>
#include <iostream>
#include <memory_resource>
#include <string>
#include <type_traits>

#include "diagnostics/Location.h"
#include "utils/BumpAllocator.h"
#include "utils/DotPrinter.h"
#include "utils/EnumMacros.h"

class Joos1WLexer;
class Joos1WParser;

namespace parsetree {

using utils::DotPrinter;

struct Node;
class Literal;
class Identifier;
class Operator;
class Modifier;
class BasicType;

/// @brief The basic type-tagged node in the parse tree.
struct Node {
   friend class ::Joos1WLexer;
   friend class ::Joos1WParser;

#define NODE_TYPE_LIST(F)               \
   /* Leaf nodes */                     \
   F(Literal)                           \
   F(QualifiedIdentifier)               \
   F(Identifier)                        \
   F(Operator)                          \
   F(BasicType)                         \
   F(Modifier)                          \
   F(ArrayType)                         \
   F(Type)                              \
   F(Poison)                            \
   /* Compilation Unit */               \
   F(CompilationUnit)                   \
   F(PackageDeclaration)                \
   F(ImportDeclarationList)             \
   F(SingleTypeImportDeclaration)       \
   F(TypeImportOnDemandDeclaration)     \
   /* Modifiers */                      \
   F(ModifierList)                      \
   /* Classes */                        \
   F(ClassDeclaration)                  \
   F(FieldDeclaration)                  \
   F(ClassBodyDeclarationList)          \
   F(ConstructorDeclaration)            \
   F(SuperOpt)                          \
   /* Interfaces */                     \
   F(InterfaceDeclaration)              \
   F(InterfaceMemberDeclarationList)    \
   F(InterfaceTypeList)                 \
   /* Methods */                        \
   F(AbstractMethodDeclaration)         \
   F(MethodHeader)                      \
   F(MethodDeclaration)                 \
   F(FormalParameterList)               \
   F(FormalParameter)                   \
   /* Statements */                     \
   F(Statement)                         \
   F(Block)                             \
   F(BlockStatementList)                \
   F(IfThenStatement)                   \
   F(WhileStatement)                    \
   F(ForStatement)                      \
   F(ReturnStatement)                   \
   F(StatementExpression)               \
   /* Variable declarations and such */ \
   F(VariableDeclarator)                \
   F(LocalVariableDeclaration)          \
   /* Expressions  */                   \
   F(Expression)                        \
   F(ArgumentList)                      \
   F(FieldAccess)                       \
   F(ArrayAccess)                       \
   F(CastExpression)                    \
   F(MethodInvocation)                  \
   F(ArrayCreationExpression)           \
   F(ClassInstanceCreationExpression)   \
   F(Dims)
public:
   /// @brief The enum for each node type
   DECLARE_ENUM(Type, NODE_TYPE_LIST)
private:
   DECLARE_STRING_TABLE(Type, type_strings, NODE_TYPE_LIST)
#undef NODE_TYPE_LIST

protected:
   /// @brief Protected constructor for leaf nodes
   /// @param type The type of the leaf node
   Node(SourceRange loc, Type type)
         : loc{loc}, type{type}, args{nullptr}, num_args{0}, parent_{nullptr} {}

   /// @brief Protected constructor for non-leaf nodes
   /// @tparam ...Args The child node types (should be Node*)
   /// @param type The type of the node
   /// @param ...args The child nodes
   template <typename... Args>
   Node(SourceRange loc, BumpAllocator& alloc, Type type, Args&&... args)
         : loc{loc},
           type{type},
           args{static_cast<Node**>(alloc.allocate_bytes(
                 sizeof...(Args) * sizeof(Node*), alignof(Node*)))},
           num_args{sizeof...(Args)} {
      static_assert(sizeof...(Args) > 0, "Must have at least one child");
      static_assert(std::conjunction_v<std::is_convertible<Args, Node*>...>,
                    "All arguments must be convertible to Node*");
      std::array<Node*, sizeof...(Args)> tmp{std::forward<Args>(args)...};
      for(size_t i = 0; i < sizeof...(Args); i++) {
         this->args[i] = tmp[i];
         if(this->args[i] != nullptr) this->args[i]->parent_ = this;
      }
   }

public:
   /// @brief Gets the number of children
   size_t num_children() const { return num_args; }
   /// @brief Gets the child at index i
   Node* child(size_t i) const { return args[i]; }
   /// @brief Gets the type of the node
   Type get_node_type() const { return type; }
   /// @brief Operator to turn Type into a string
   std::string type_string() const { return Type_to_string(type, "Unknown Type"); }
   /// @brief Operator to turn Type into a string
   static std::string type_string(Type type) {
      return Type_to_string(type, "Unknown Type");
   }
   /// @brief Check if the tree has been poisoned
   bool is_poisoned() const {
      if(type == Type::Poison)
         return true;
      else {
         for(size_t i = 0; i < num_args; i++) {
            if(args[i] == nullptr) continue;
            if(args[i]->is_poisoned()) return true;
         }
         return false;
      }
   }
   /// @brief Get the location of the node
   SourceRange location() const { return loc; }
   /// @brief Get the parent of the node
   Node const* parent() const { return parent_; }
   Node* parent() { return parent_; }
   void mark() { marked = true; }
   bool is_marked() const { return marked; }

public:
   /// @brief Virtual function to print the node
   virtual std::ostream& print(std::ostream& os) const;
   /// @brief Print the node as a dot file
   std::ostream& printDot(std::ostream& os) const;

protected:
   /// @brief Custom function to print the DOT node
   virtual void printDotNode(DotPrinter& dp) const;

private:
   /// @brief Recursively print the DOT graph
   int printDotRecursive(DotPrinter& dp, const Node& node) const;

private:
   SourceRange loc;
   Type type;
   Node** args;
   size_t num_args;
   Node* parent_;
   bool marked = false;
};

// Output stream operator for a parse tree node
std::ostream& operator<<(std::ostream& os, Node const& n);

////////////////////////////////////////////////////////////////////////////////

/// @brief A lex node in the parse tree representing a literal value.
class Literal : public Node {
   friend class ::Joos1WLexer;
   friend class ::Joos1WParser;

#define LITERAL_TYPE_LIST(F) \
   F(Integer)                \
   F(Character)              \
   F(String)                 \
   F(Boolean)                \
   F(Null)
public:
   /// @brief The enum for each literal type
   DECLARE_ENUM(Type, LITERAL_TYPE_LIST)
private:
   DECLARE_STRING_TABLE(Type, literal_strings, LITERAL_TYPE_LIST)
#undef LITERAL_TYPE_LIST

private:
   Literal(SourceRange loc, BumpAllocator const& alloc, Type type,
           char const* value)
         : Node{loc, Node::Type::Literal},
           type{type},
           isNegative{false},
           value{value, alloc} {}

public:
   // Override printing for this leaf node
   std::ostream& print(std::ostream& os) const override;
   // Set the value of the literal to negative
   void setNegative() { isNegative = true; }
   // Check if the literal is valid
   bool isValid() const;
   // Get the type of the literal
   Type get_type() const { return type; }
   // Get the string representation of the literal
   std::string_view get_value() const { return value; }

protected:
   void printDotNode(DotPrinter& dp) const override;

private:
   Type type;
   bool isNegative;
   std::pmr::string value;
};

////////////////////////////////////////////////////////////////////////////////

/// @brief A lex node in the parse tree representing an identifier.
class Identifier : public Node {
   friend class ::Joos1WLexer;
   friend class ::Joos1WParser;

private:
   Identifier(SourceRange loc, BumpAllocator const& alloc, char const* name)
         : Node{loc, Node::Type::Identifier}, name{name, alloc} {}

public:
   // Get the name of the identifier
   std::string_view get_name() const { return name; }
   // Override printing for this leaf node
   std::ostream& print(std::ostream& os) const override;

protected:
   void printDotNode(DotPrinter& dp) const override;

private:
   std::pmr::string name;
};

////////////////////////////////////////////////////////////////////////////////

/// @brief A lex node in the parse tree representing an operator.
class Operator : public Node {
   friend class ::Joos1WLexer;
   friend class ::Joos1WParser;

public:
   enum class Type {
      Assign,
      GreaterThan,
      LessThan,
      Not,
      Equal,
      LessThanOrEqual,
      GreaterThanOrEqual,
      NotEqual,
      And,
      Or,
      BitwiseAnd,
      BitwiseOr,
      BitwiseXor,
      BitwiseNot,
      Add,
      Subtract,
      Multiply,
      Divide,
      Modulo,
      Plus,
      Minus,
      InstanceOf
   };

private:
   Operator(SourceRange loc, Type type)
         : Node{loc, Node::Type::Operator}, type{type} {}

public:
   // Get the type of the operator
   std::ostream& print(std::ostream& os) const override {
      return os << to_string();
   }
   // Get the string representation of the operator
   std::string to_string() const;

   Type get_type() const { return type; }

protected:
   void printDotNode(DotPrinter& dp) const override;

private:
   Type type;
};

////////////////////////////////////////////////////////////////////////////////

/// @brief A lex node in the parse tree representing a modifier.
class Modifier : public Node {
   friend class ::Joos1WLexer;
   friend class ::Joos1WParser;

#define MODIFIER_TYPE_LIST(F) \
   F(Public)                  \
   F(Protected)               \
   F(Static)                  \
   F(Abstract)                \
   F(Final)                   \
   F(Native)
public:
   /// @brief The enum for each modifier type
   DECLARE_ENUM(Type, MODIFIER_TYPE_LIST)
private:
   DECLARE_STRING_TABLE(Type, modifier_strings, MODIFIER_TYPE_LIST)
#undef MODIFIER_TYPE_LIST

private:
   Modifier(SourceRange loc, Type type)
         : Node{loc, Node::Type::Modifier}, modty{type} {}

public:
   // Get the type of the modifier
   Type get_type() const { return modty; }
   // Print the string representation of the modifier
   std::ostream& print(std::ostream& os) const override;

protected:
   void printDotNode(DotPrinter& dp) const override;

private:
   Type modty;
};

////////////////////////////////////////////////////////////////////////////////

/// @brief A lex node in the parse tree representing a basic type.
class BasicType : public Node {
   friend class ::Joos1WLexer;
   friend class ::Joos1WParser;

#define BASIC_TYPE_LIST(F) \
   F(Byte)                 \
   F(Short)                \
   F(Int)                  \
   F(Char)                 \
   F(Boolean)
public:
   /// @brief The enum for each basic type
   DECLARE_ENUM(Type, BASIC_TYPE_LIST)
private:
   DECLARE_STRING_TABLE(Type, basic_type_strings, BASIC_TYPE_LIST)
#undef BASIC_TYPE_LIST

private:
   BasicType(SourceRange loc, Type type)
         : Node{loc, Node::Type::BasicType}, type{type} {}

public:
   // Get the type of the basic type
   Type get_type() const { return type; }
   // Print the string representation of the basic type
   std::ostream& print(std::ostream& os) const override;

protected:
   void printDotNode(DotPrinter& dp) const override;

private:
   Type type;
};

} // namespace parsetree