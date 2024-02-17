#pragma once

#include <cassert>

#include "ast/AstNode.h"
#include "parsetree/ParseTree.h"
#include "semantic/NameResolver.h"
#include "utils/EnumMacros.h"

namespace ast {

class ClassDecl;
class InterfaceDecl;

/// @brief Represents a primitive type in the Java language.
class BuiltInType final : public Type {
public:
#define BASIC_TYPE_LIST(F) \
   F(Void)                 \
   F(Byte)                 \
   F(Short)                \
   F(Int)                  \
   F(Char)                 \
   F(Boolean)
   DECLARE_ENUM(Kind, BASIC_TYPE_LIST)
private:
   DECLARE_STRING_TABLE(Kind, kind_strings, BASIC_TYPE_LIST)
#undef BASIC_TYPE_LIST

private:
   Kind kind;

public:
   explicit BuiltInType(Kind kind) : kind{kind} {}
   explicit BuiltInType(parsetree::BasicType::Type type) {
      switch(type) {
         case parsetree::BasicType::Type::Byte:
            kind = Kind::Byte;
            break;
         case parsetree::BasicType::Type::Short:
            kind = Kind::Short;
            break;
         case parsetree::BasicType::Type::Int:
            kind = Kind::Int;
            break;
         case parsetree::BasicType::Type::Char:
            kind = Kind::Char;
            break;
         case parsetree::BasicType::Type::Boolean:
            kind = Kind::Boolean;
            break;
         default:
            break;
      }
   }
   Kind getKind() const { return kind; }
   string_view toString() const override { return Kind_to_string(kind, "??"); }
   bool isResolved() const override { return true; }
   bool operator== (const Type& other) const override { 
      if (auto otherBuiltIn = dynamic_cast<const BuiltInType*>(&other)) {
         return kind == otherBuiltIn->kind;
      }
      return false;
   }
};

/**
 * @brief Represents a reference type. This is a type that refers to either
 * a class or an interface declaration. Creation of a reference type must
 * result in a declaration being resolved to it.
 */
class ReferenceType : public Type {
protected:
   /// @brief Only used by unresolved types.
   ReferenceType() : decl_{nullptr} {}

public:
   /// @brief Reference types must be resolved when created like this
   /// @param decl The declaration that this reference type refers to.
   explicit ReferenceType(Decl* decl) : decl_{decl} {
      assert(decl && "Declaration cannot be null for reference type");
   }
   /// @brief TODO: Implement this.
   virtual string_view toString() const override { return "ReferenceType"; }
   /// @brief The reference type is resolved if it has a declaration.
   bool isResolved() const override { return decl_ != nullptr; }
   auto decl() const { return decl_; }
   /// @brief This does nothing as a reference type is always resolved.
   virtual void resolve(semantic::NameResolver&) override {
      assert(isResolved() && "Type is not resolved");
   }

   /// @brief Resolves the type to a declaration if it is an unresolved type.
   void resolveInternal(Decl* decl) {
      assert(!isResolved() && "Type already resolved");
      decl_ = decl;
   }

   bool operator== (const Type& other) const override {
      if (auto otherBuiltIn = dynamic_cast<const ReferenceType*>(&other)) {
         return decl_ == otherBuiltIn->decl_;
      }
      
      return false;
   }

protected:
   Decl* decl_;
};

/**
 * @brief Represents an unresolved reference type. This means that the
 * underlying reference type is not yet resolved to a declaration. This class
 * is used to represent types whose names are qualified with multiple parts,
 * or simple type names.
 */
class UnresolvedType : public ReferenceType {
   BumpAllocator& alloc;
   pmr_vector<std::pmr::string> identifiers;
   mutable std::pmr::string canonicalName;
   mutable bool locked_ = false;

public:
   explicit UnresolvedType(BumpAllocator& alloc)
         : ReferenceType{},
           alloc{alloc},
           identifiers{alloc},
           canonicalName{alloc} {}

   /// @brief Adds a simple name to the unresolved type.
   void addIdentifier(string_view identifier) {
      assert(!locked_ && "Cannot add identifiers to a locked unresolved type");
      identifiers.emplace_back(identifier);
   }

   /// @brief Converts the unresolved type to a string by concatenating
   /// all the simple name parts with a '.'.
   string_view toString() const override {
      if(!canonicalName.empty()) {
         return canonicalName;
      }
      for(auto& id : identifiers) {
         canonicalName += id;
         canonicalName += ".";
      }
      canonicalName.pop_back();
      return canonicalName;
   }

   /// @brief The individual parts (simple names) of the unresolved type.
   auto parts() const { return std::views::all(identifiers); }

   /// @brief Marks the unresolved type as immutable. This allows us to
   /// grab references to the parts of the unresolved type without
   /// worrying about the references being invalidated.
   void lock() const { locked_ = true; }

   /// @brief Resolves the underlying reference type to a declaration.
   void resolve(semantic::NameResolver& x) override { x.ResolveType(this); }

   std::ostream& operator<<(std::ostream& os) const { return os << toString(); }
};

/// @brief Represents an (unsized) array type.
class ArrayType final : public Type {
   Type* elementType;
   std::pmr::string name;

public:
   ArrayType(BumpAllocator& alloc, Type* elementType)
         : elementType{elementType}, name{elementType->toString(), alloc} {
      name += "[]";
   }
   string_view toString() const override { return name; }
   bool isResolved() const override { return elementType->isResolved(); }
   void resolve(semantic::NameResolver& x) override {
      // Resolve only if the element type is an unresolved type.
      if(auto unresTy = dynamic_cast<UnresolvedType*>(elementType)) {
         if(elementType->isResolved()) unresTy->resolve(x);
      }
   }
   bool operator== (const Type& other) const override {
      if (auto otherArrayType = dynamic_cast<const ArrayType*>(&other)) {
         return *elementType == *otherArrayType->elementType;
      }
      return false;
   }
};

} // namespace ast
