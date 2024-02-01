#pragma once

#include <string>
#include <vector>

namespace ast {

// Forward delcarations of AST nodes

class CompilationUnit;
class PackageDeclaration;
class ImportDeclarations;
class Import;
class TypeDeclarations;

class QualifiedIdentifier;

class Identifier;

class AstNode {
    // ...
};

class CompilationUnit : public AstNode {
    PackageDeclaration* packageDeclaration;
    ImportDeclarations* importDeclarations;
    TypeDeclarations* typeDeclarations;
public:
    CompilationUnit(
        PackageDeclaration* packageDeclaration,
        ImportDeclarations* importDeclarations,
        TypeDeclarations* typeDeclarations
    ) : packageDeclaration{packageDeclaration}, importDeclarations{importDeclarations}, typeDeclarations{typeDeclarations} {}
};

class PackageDeclaration : public AstNode {
    QualifiedIdentifier* qualifiedIdentifier;
public:
    PackageDeclaration(
        QualifiedIdentifier* qualifiedIdentifier
    ) : qualifiedIdentifier{qualifiedIdentifier} {}
};

class ImportDeclarations : public AstNode {
    std::vector<Import *> imports;
public:
    ImportDeclarations(
        std::vector<Import *> imports
    ): imports{imports} {}
};

class Import : public AstNode {
    bool isSingleType;
    QualifiedIdentifier* qualifiedIdentifier;
public:
    Import(
        bool isSingleType,
        QualifiedIdentifier* qualifiedIdentifier
    ): isSingleType{isSingleType}, qualifiedIdentifier{qualifiedIdentifier} {}
};

class TypeDeclarations : public AstNode {
public:

};

class QualifiedIdentifier : public AstNode {
    std::vector<Identifier*> identifiers;
public:
    QualifiedIdentifier(
        std::vector<Identifier*> identifiers
    ): identifiers {identifiers} {};
};


/* ========================================================================== */
/*                               Literals                                     */
/* ========================================================================== */

class Identifier : public AstNode {
    std::string value;
public:
    Identifier(
        std::string value
    ) : value{value} {}
};

} // namespace ast
