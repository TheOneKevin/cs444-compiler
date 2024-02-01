#pragma once

#include <string>
#include <vector>
#include <iostream>

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
public:
    virtual std::ostream& print(std::ostream& os) const;
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

    std::ostream& print(std::ostream& os) const;
};

class PackageDeclaration : public AstNode {
    QualifiedIdentifier* qualifiedIdentifier;
public:
    PackageDeclaration(
        QualifiedIdentifier* qualifiedIdentifier
    ) : qualifiedIdentifier{qualifiedIdentifier} {}

    std::ostream& print(std::ostream& os) const;
};

class ImportDeclarations : public AstNode {
    std::vector<Import *> imports;
public:
    ImportDeclarations(
        std::vector<Import *> imports
    ): imports{imports} {}

    std::ostream& print(std::ostream& os) const;
};

class Import : public AstNode {
    bool isSingleType;
    QualifiedIdentifier* qualifiedIdentifier;
public:
    Import(
        bool isSingleType,
        QualifiedIdentifier* qualifiedIdentifier
    ): isSingleType{isSingleType}, qualifiedIdentifier{qualifiedIdentifier} {}

    std::ostream& print(std::ostream& os) const;
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

    std::ostream& print(std::ostream& os) const;
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

    std::ostream& print(std::ostream& os) const;
};

std::ostream& operator<< (std::ostream& os, const AstNode& astNode);

} // namespace ast
