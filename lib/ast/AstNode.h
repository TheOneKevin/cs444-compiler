#pragma once

#include <iostream>
#include <string>
#include <vector>

#include "parsetree/ParseTree.h"
#include "utils/DotPrinter.h"

namespace ast {

using utils::DotPrinter;

template <typename T>
using pmr_vector = std::pmr::vector<T>;
template <typename T>
using array_ref = std::pmr::vector<T>&;
using std::string_view;

// Base class for all AST nodes ////////////////////////////////////////////////

class Type;
class Decl;
class DeclContext;
class Stmt;

/// @brief Base class for all AST nodes. Helps unify printing and dot printing.
class AstNode {
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
         : name{name, alloc} {}
   std::string_view getName() const { return name; }
   virtual int printDotNode(DotPrinter& dp) const override = 0;

private:
   std::pmr::string name;
};

/// @brief Base class for all declaration contexts (i.e., methods).
class DeclContext : public AstNode {
public:
   virtual int printDotNode(DotPrinter& dp) const override = 0;
};

/// @brief Base class for all types.
class Type : public AstNode {
public:
   virtual std::string toString() const = 0;
   std::ostream& print(std::ostream& os, int indentation = 0) const override {
      return os << indent(indentation) << toString();
   }
   int printDotNode(DotPrinter& dp) const override {
      int id = dp.id();
      dp.printLabel(id, toString());
      return id;
   }
};

/// @brief Base class for all statements.
class Stmt : public AstNode {
public:
   virtual int printDotNode(DotPrinter& dp) const override = 0;
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
   requires std::is_convertible_v<std::ranges::range_value_t<Range>,
                                  ast::AstNode*>
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

// Other classes ///////////////////////////////////////////////////////////////

class Modifiers {
   bool isPublic_;
   bool isProtected_;
   bool isStatic_;
   bool isFinal_;
   bool isAbstract_;
   bool isNative_;

public:
   void set(parsetree::Modifier modifier);

   void set(ast::Modifiers modifier);

   bool isPublic() const { return isPublic_; }
   bool isProtected() const { return isProtected_; }
   bool isStatic() const { return isStatic_; }
   bool isFinal() const { return isFinal_; }
   bool isAbstract() const { return isAbstract_; }
   bool isNative() const { return isNative_; }

   void setPublic();
   void setProtected();
   void setStatic();
   void setFinal();
   void setAbstract();
   void setNative();

   std::string toString() const;
};

class QualifiedIdentifier {
   std::vector<std::string> identifiers;

public:
   void addIdentifier(std::string identifier) {
      identifiers.push_back(identifier);
   }
   std::string toString() const {
      std::string result;
      for(auto& identifier : identifiers) {
         result += identifier;
         result += ".";
      }
      result.pop_back();
      return result;
   }
   std::ostream& operator<<(std::ostream& os) const { return os << toString(); }
};

} // namespace ast
