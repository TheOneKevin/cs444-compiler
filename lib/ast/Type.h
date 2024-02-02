#include "ast/AstNode.h"
#include "utils/EnumMacros.h"

namespace ast {

class BuiltInType : public Type {
public:
    #define BASIC_TYPE_LIST(F) \
        F(Void) \
        F(Byte) \
        F(Short) \
        F(Int) \
        F(Char) \
        F(Boolean)
    DECLARE_ENUM(Kind, BASIC_TYPE_LIST)
private:
    DECLARE_STRING_TABLE(Kind, kind_strings, BASIC_TYPE_LIST)
    #undef BASIC_TYPE_LIST

private:
    Kind kind;
public:
    BuiltInType(Kind kind): kind{kind} {}
    Kind getKind() const { return kind; }
    std::ostream& print(std::ostream& os, int indentation = 0) const override;
};

class ArrayType : public Type {
    Type* elementType;
};

class ReferenceType : public Type {
    QualifiedIdentifier identifiers;
};

} // namespace ast
