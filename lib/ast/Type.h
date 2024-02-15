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
   std::string toString() const override { return Kind_to_string(kind, "??"); }
};

class ArrayType : public Type {
   Type* elementType;

public:
   ArrayType(Type* elementType) : elementType{elementType} {}
   std::string toString() const override {
      return elementType->toString() + "[]";
   }
};

class ReferenceType : public Type {
public:
   std::string toString() const override {
      return "Unknown";
   }
};

class UnresolvedType : public ReferenceType {
   std::vector<std::string> identifiers;

public:
   UnresolvedType(BumpAllocator& a) {}

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
