#pragma once

#include <cassert>

#include "ast/AstNode.h"
#include "parsetree/ParseTree.h"
#include "utils/EnumMacros.h"
#include "semantic/NameResolver.h"

namespace ast {

class ClassDecl;
class InterfaceDecl;

/// @brief Represents a primitive type in the Java language.
class BuiltInType final : public Type {
public:
#define BASIC_TYPE_LIST(F) \
   F(NoneType)             \
   F(Byte)                 \
   F(Short)                \
   F(Int)                  \
   F(Char)                 \
   F(Boolean)              \
   F(String)               
   DECLARE_ENUM(Kind, BASIC_TYPE_LIST)
private:
   DECLARE_STRING_TABLE(Kind, kind_strings, BASIC_TYPE_LIST)
#undef BASIC_TYPE_LIST

private:
   Kind kind;

public:
   BuiltInType(Kind kind, SourceRange loc) : Type{loc}, kind{kind} {}
   BuiltInType(parsetree::BasicType::Type type, SourceRange loc = {}) : Type{loc} {
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
            assert(false && "Invalid basic type");
      }
   }
   BuiltInType(parsetree::Literal::Type type) : Type{SourceRange{}} {
      switch(type) {
         case parsetree::Literal::Type::Integer:
            kind = Kind::Int;
            break;
         case parsetree::Literal::Type::Character:
            kind = Kind::Char;
            break;
         case parsetree::Literal::Type::String:
            kind = Kind::String;
            break;
         case parsetree::Literal::Type::Boolean:
            kind = Kind::Boolean;
            break;
         case parsetree::Literal::Type::Null:
            kind = Kind::NoneType;
            break;
         default:
            assert(false && "Invalid literal type");
      }
   }
   Kind getKind() const { return kind; }
   string_view toString() const override { return Kind_to_string(kind, "??"); }
   bool isResolved() const override { return true; }
   bool operator==(const Type& other) const override {
      if(auto otherBuiltIn = dynamic_cast<const BuiltInType*>(&other)) {
         return kind == otherBuiltIn->kind;
      }
      return false;
   }
   bool isNumeric() const override { 
      return kind != BuiltInType::Kind::Boolean; 
   }
   bool isBoolean() const override { return kind == BuiltInType::Kind::Boolean; }
   bool isNull() const override { return kind == BuiltInType::Kind::NoneType; }
   bool isString() const override { return kind == BuiltInType::Kind::String; }
};

/**
 * @brief Represents a reference type. This is a type that refers to either
 * a class or an interface declaration. Creation of a reference type must
 * result in a declaration being resolved to it.
 */
class ReferenceType : public Type {
protected:
   /// @brief Only used by unresolved types.
   ReferenceType(SourceRange loc) : Type{loc}, decl_{nullptr} {}

public:
   /// @brief Reference types must be resolved when created like this
   /// @param decl The declaration that this reference type refers to.
   ReferenceType(Decl* decl, SourceRange loc) : Type{loc}, decl_{decl} {
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

   bool operator==(const Type& other) const override final {
      if(auto otherType = dynamic_cast<const ReferenceType*>(&other)) {
         assert(decl_ && otherType->decl_ &&
                "Reference types are not resolved during comparison");
         return decl_ == otherType->decl_;
      }
      return false;
   }
   bool isNumeric() const override final { 
      if (auto builtIn = getAsBuiltIn()) {
         return builtIn->isNumeric();
      }
      return false;
   }
   bool isBoolean() const override final { return false; }
   bool isNull() const override { return false; }
   bool isString() const override final {
      if (auto builtIn = getAsBuiltIn()) {
         return builtIn->isString();
      }
      return false;
   }

   /**
    * @brief If the current reference type is a built-in type, then returns
    * the built-in type. For ex. if the reference type is java.lang.Integer,
    * this will return the "int" built-in type (ast::Node).
    * 
    * @return ast::BuiltInType* Returns the built-in type if the reference type
    * is a built-in type. Otherwise, returns nullptr.
    */
   ast::BuiltInType* getAsBuiltIn() const {
      // FIXME(everyone): Should this be part of the operator==?
      return nullptr;
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
class UnresolvedType final : public ReferenceType {
   BumpAllocator& alloc;
   pmr_vector<std::pmr::string> identifiers;
   mutable std::pmr::string canonicalName;
   mutable bool locked_ = false;
   bool valid_ = true;

public:
   explicit UnresolvedType(BumpAllocator& alloc, SourceRange loc)
         : ReferenceType{loc},
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
      if(identifiers.empty()) return "";
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
   void resolve(semantic::NameResolver& x) override {
      assert(!isInvalid() && "Attempted to resolve invalid type");
      x.ResolveType(this);
   }

   /// @brief Returns if the type is invalid now
   bool isInvalid() const override { return !valid_; }
   void invalidate() { valid_ = false; }

   std::ostream& operator<<(std::ostream& os) const { return os << toString(); }
};

/// @brief Represents an (unsized) array type.
class ArrayType final : public Type {
   Type* elementType;
   std::pmr::string name;

public:
   ArrayType(BumpAllocator& alloc, Type* elementType, SourceRange loc = {})
         : Type{loc},
           elementType{elementType},
           name{elementType->toString(), alloc} {
      name += "[]";
   }
   string_view toString() const override { return name; }
   bool isResolved() const override { return elementType->isResolved(); }
   void resolve(semantic::NameResolver& x) override {
      // Resolve only if the element type is an unresolved type.
      if(auto unresTy = dynamic_cast<UnresolvedType*>(elementType)) {
         if(!elementType->isResolved()) unresTy->resolve(x);
      }
   }
   bool operator==(const Type& other) const override {
      if(auto otherArrayType = dynamic_cast<const ArrayType*>(&other)) {
         return *elementType == *otherArrayType->elementType;
      }
      return false;
   }
   Type* getElementType() const { return elementType; }
   bool isNumeric() const override { return false; }
   bool isBoolean() const override { return false; }
   bool isNull() const override { return false; }
   bool isString() const override { return false; }
};


class MethodType final : public Type {
   Type* returnType;
   pmr_vector<Type*> paramTypes;

public:
   MethodType(Type* returnType, array_ref<Type*> paramTypes, SourceRange loc = {})
         : Type{loc}, returnType{returnType}, paramTypes{paramTypes} {}

   string_view toString() const override { 
      return "MethodType"; 
   }
   bool isResolved() const override { return true; }

   bool operator==(const Type& other) const override {
      if(auto otherArrayType = dynamic_cast<const MethodType*>(&other)) {
         if (*returnType != *otherArrayType->returnType) {
            return false;
         }
         if (paramTypes.size() != otherArrayType->paramTypes.size()) {
            return false;
         }
         for (size_t i = 0; i < paramTypes.size(); i++) {
            if (*paramTypes[i] != *otherArrayType->paramTypes[i]) {
               return false;
            }
         }
         return true;
      }
      return false;
   }

   Type* getReturnType() const { return returnType; }
   pmr_vector<Type*> getParamTypes() const { return paramTypes; }

   bool isNumeric() const override { return false; }
   bool isBoolean() const override { return false; }
   bool isNull() const override { return false; }
   bool isString() const override { return false; }
};
} // namespace ast
