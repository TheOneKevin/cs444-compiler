#pragma once

#include "ast/AstNode.h"
#include "parsetree/ParseTree.h"
#include "utils/EnumMacros.h"

namespace ast {

class BuiltInType : public Type {
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
   BuiltInType(Kind kind) : kind{kind} {}
   BuiltInType(parsetree::BasicType::Type type) {
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
};

class ArrayType : public Type {
   Type* elementType;
   std::pmr::string name;

public:
   ArrayType(BumpAllocator& alloc, Type* elementType)
         : elementType{elementType}, name{elementType->toString(), alloc} {
      name += "[]";
   }
   string_view toString() const override { return name; }
};

class ReferenceType : public Type {
public:
   string_view toString() const override { return "Unknown"; }
};

class UnresolvedType : public ReferenceType {
   BumpAllocator& alloc;
   pmr_vector<std::pmr::string> identifiers;
   mutable std::pmr::string canonicalName;

public:
   UnresolvedType(BumpAllocator& alloc)
         : alloc{alloc}, identifiers{alloc}, canonicalName{alloc} {}

   void addIdentifier(string_view identifier) {
      identifiers.emplace_back(identifier);
   }

   string_view toString() const {
      for(auto& id : identifiers) {
         canonicalName += id;
         canonicalName += ".";
      }
      canonicalName.pop_back();
      return canonicalName;
   }

   std::ostream& operator<<(std::ostream& os) const { return os << toString(); }
};

} // namespace ast
