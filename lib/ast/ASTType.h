#include "AST.h"

namespace ast {

class BuiltInType : public Type {
    enum class Kind {
        VOID,
        BOOLEAN,
        BYTE,
        CHAR,
        SHORT,
        INT,
    };
public:
    BuiltInType(Kind kind): kind{kind} {}
};

class ArrayType : public Type {
    Type* elementType;
};

class ReferenceType : public Type {
    QualifiedIdentifier identifiers;
};

} // namespace ast
