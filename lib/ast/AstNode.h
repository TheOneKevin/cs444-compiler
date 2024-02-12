#pragma once

#include <iostream>
#include <string>
#include <vector>

#include "parsetree/ParseTree.h"

namespace ast {

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

static std::string indent(int indentation) {
   return std::string(indentation * 2, ' ');
}

class AstNode {
public:
   virtual std::ostream& print(std::ostream& os, int indentation = 0) const = 0;
};

class Decl : public AstNode {
public:
   Decl(BumpAllocator& alloc, std::string_view name) noexcept
         : name{name, alloc} {}
   std::string_view getName() const { return name; }

private:
   std::pmr::string name;
};

class DeclContext : public AstNode {};

class Type : public AstNode {
public:
   virtual std::string toString() const = 0;
   std::ostream& print(std::ostream& os, int indentation = 0) const override {
      return os << indent(indentation) << toString();
   }
};

class Stmt : public AstNode {};

std::ostream& operator<<(std::ostream& os, const AstNode& astNode);

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
