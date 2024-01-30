%parse-param {int *ret}

%code top {
    #include <stdio.h>

    extern int yylex(void);

    static void yyerror(int *ret, const char* s) {
        fprintf(stderr, "%s\n", s);
    }
}

// Tokens and whatnot

// yylval is a union of this type

%code requires {
    #include "parsetree/ParseTreeTypes.h"
}

%union {
    struct parsetree::Literal *lit;
    struct parsetree::Identifier *id;
}

%token<lit> IntegerLiteral;
%token<lit> BooleanLiteral;
%token<lit> StringLiteral;
%token<lit> CharacterLiteral;
%token<lit> NullLiteral;
%token<id> Identifier;

%token OperatorAssign;
%token OperatorGreaterThan;
%token OperatorLessThan;
%token OperatorNot;
%token OperatorEqual;
%token OperatorLessThanOrEqual;
%token OperatorGreaterThanOrEqual;
%token OperatorNotEqual;
%token OperatorAnd;
%token OperatorOr;
%token OperatorBitwiseAnd;
%token OperatorBitwiseOr;
%token OperatorIncrement;
%token OperatorDecrement;
%token OperatorPlus;
%token OperatorMinus;
%token OperatorMultiply;
%token OperatorDivide;
%token OperatorModulo;
%token OperatorXor;
%token OperatorBitwiseNot;

%token SeparatorLeftParenthesis;
%token SeparatorRightParenthesis;
%token SeparatorLeftBrace;
%token SeparatorRightBrace;
%token SeparatorLeftBracket;
%token SeparatorRightBracket;
%token SeparatorSemicolon;
%token SeparatorComma;
%token SeparatorDot;

%token KeywordAbstract;
%token KeywordBoolean;
%token KeywordByte;
%token KeywordChar;
%token KeywordClass;
%token KeywordElse;
%token KeywordExtends;
%token KeywordFinal;
%token KeywordFor;
%token KeywordIf;
%token KeywordImplements;
%token KeywordImport;
%token KeywordInstanceof;
%token KeywordInt;
%token KeywordInterface;
%token KeywordNative;
%token KeywordNew;
%token KeywordPackage;
%token KeywordProtected;
%token KeywordPublic;
%token KeywordReturn;
%token KeywordShort;
%token KeywordStatic;
%token KeywordThis;
%token KeywordVoid;
%token KeywordWhile;
%token Comment;

%precedence KeywordIf
%precedence KeywordElse

%%

/* ========================================================================== */
/*                               Grammar rules                                */
/* ========================================================================== */

start
    : CompilationUnit
    ;

CompilationUnit
    : PackageDeclaration ImportDeclarations TypeDeclarations
    ;

PackageDeclaration
    : /* empty */
    | KeywordPackage QualifiedIdentifier SeparatorSemicolon
    ;

ImportDeclarations
    : /* empty */
    | ImportDeclarations ImportDeclaration
    ;

ImportDeclaration
    : KeywordImport QualifiedIdentifier SeparatorSemicolon
    ;

TypeDeclarations
    : /* empty */
    | TypeDeclarations TypeDeclaration
    ;

TypeDeclaration
    : ClassOrInterfaceDeclaration
    ;

/* ========================================================================== */
/*                          Class declarations                                */
/* ========================================================================== */

ClassOrInterfaceDeclaration
    : ModifiersOpt ClassDeclaration
    | ModifiersOpt InterfaceDeclaration
    ;

ClassDeclaration
    : KeywordClass Identifier ClassBody
    | KeywordClass Identifier KeywordExtends Type ClassBody
    | KeywordClass Identifier KeywordImplements Type ClassBody
    | KeywordClass Identifier KeywordExtends Type KeywordImplements TypeList ClassBody
    ;
    
ClassBody
    : SeparatorLeftBrace ClassBodyList SeparatorRightBrace
    ;

ClassBodyList
    : /* empty */
    | ClassBodyList ClassBodyDeclaration
    ;

ClassBodyDeclaration
    : SeparatorSemicolon
    | ModifiersOpt MemberDecl
    ;

/* ========================================================================== */
/*                          Interface declarations                            */
/* ========================================================================== */

InterfaceDeclaration
    : KeywordInterface Identifier InterfaceBody
    | KeywordInterface Identifier KeywordExtends TypeList InterfaceBody
    ;

InterfaceBody
    : SeparatorLeftBrace InterfaceBodyDeclList SeparatorRightBrace
    ;

InterfaceBodyDeclList
    : /* empty */
    | InterfaceBodyDeclList InterfaceBodyDeclaration
    ;

InterfaceBodyDeclaration
    : SeparatorSemicolon
    | ModifiersOpt InterfaceMemberDecl
    ;

InterfaceMemberDecl
    : Type Identifier FormalParameters SeparatorSemicolon
    | KeywordVoid Identifier FormalParameters SeparatorSemicolon
    ;

/* ========================================================================== */
/*                              Member declarations                           */
/* ========================================================================== */

MemberDecl
    : MethodOrFieldDecl
    | KeywordVoid Identifier MethodDeclaratorRest
	| Identifier ConstructorDeclaratorRest
    ;

MethodOrFieldDecl
    : Type Identifier MethodOrFieldRest
    ;

MethodOrFieldRest
    : VariableDeclaratorRest
    | MethodDeclaratorRest
    ;

MethodDeclaratorRest
    : FormalParameters Block
    | FormalParameters SeparatorSemicolon
    ;

ConstructorDeclaratorRest
    : FormalParameters Block
    ;

FormalParameters
    : SeparatorLeftParenthesis SeparatorRightParenthesis
    | SeparatorLeftParenthesis FormalParameterList SeparatorRightParenthesis
    ;

FormalParameterList
    : FormalParameter
    | FormalParameterList SeparatorComma FormalParameter
    ;

FormalParameter
    : Type VariableDeclaratorId
    ;

/* ========================================================================== */
/*                              Block declarations                            */
/* ========================================================================== */

Block
    : SeparatorLeftBrace BlockStatements SeparatorRightBrace
    ;

BlockStatements
    : /* empty */
    | BlockStatements BlockStatement
    ;

BlockStatement
    : LocalVariableDeclarationStatement
    | Statement
    ;

/* ========================================================================== */
/*                           Variable declarations                            */
/* ========================================================================== */

LocalVariableDeclarationStatement
    : Type VariableDeclarators SeparatorSemicolon
    ;

VariableDeclaratorId
    : Identifier
    | VariableDeclaratorId SeparatorLeftBracket SeparatorRightBracket
    ;

VariableDeclarators
    : VariableDeclarator
    | VariableDeclarators SeparatorComma VariableDeclarator
    ;

VariableDeclarator
    : Identifier VariableDeclaratorRest
    ;
    
VariableDeclaratorRest
    : /* empty */
    | SeparatorLeftBracket SeparatorRightBrace
    | SeparatorLeftBracket SeparatorRightBrace OperatorAssign VariableInitializer  
    ;  

VariableInitializer
    : ArrayInitializer
    | Expression
    ;

ArrayInitializer
    : SeparatorLeftBrace ArrayInitializerList SeparatorRightBrace
    ;

ArrayInitializerList
    : Expression
    | ArrayInitializerList SeparatorComma Expression
    ;

/* ========================================================================== */
/*                          Statement declarations                            */
/* ========================================================================== */

Statement
    : Block
    | KeywordIf ParExpression Statement
    | KeywordIf ParExpression Statement KeywordElse Statement
    | KeywordFor SeparatorLeftParenthesis
        LocalVariableDeclarationStatement
        Expression SeparatorRightParenthesis
        Statement
    | KeywordFor SeparatorLeftParenthesis
        LocalVariableDeclarationStatement
        Expression SeparatorSemicolon
        Expression SeparatorRightParenthesis
        Statement
    | KeywordWhile ParExpression Statement
    | KeywordReturn SeparatorSemicolon
    | KeywordReturn Expression SeparatorSemicolon
    | SeparatorSemicolon
    | Expression SeparatorSemicolon
    | Identifier SeparatorSemicolon
    ;

/* ========================================================================== */
/*                          Type declarations                                 */
/* ========================================================================== */

TypeList
    : Type
    | TypeList SeparatorComma Type
    ;

Type
    : QualifiedIdentifier
    | QualifiedIdentifier SeparatorLeftBracket SeparatorRightBracket
    | BasicType
    ;

BasicType
    : KeywordBoolean
    | KeywordByte
    | KeywordChar
    | KeywordShort
    | KeywordInt
    ;

/* ========================================================================== */
/*                      Expressions and statements                            */
/* ========================================================================== */

Expression
    : Expression2
    | Expression2 OperatorAssign Expression2
    ;

Expression2
    : Expression3
    | Expression3 Expression2Rest
    ;

Expression2Rest
    : Expression2Rest InfixOp Expression3
    | Expression3 KeywordInstanceof Type
    ;

Expression3
    : PrefixOp Expression3
    | SeparatorLeftParenthesis Type SeparatorRightParenthesis Expression3
    | Primary
    | PrimaryNoNewArray Selector
    | PrimaryNoNewArray Selector PostfixOp
    ;

ParExpression
    : SeparatorLeftParenthesis Expression SeparatorRightParenthesis
    ;

Primary
    : PrimaryNoNewArray
    | NewArrayExpr
    ;

PrimaryNoNewArray
    : ParExpression
    | KeywordThis
    | Literal
    | NewObjectExpr
    | QualifiedIdentifier Identifier
    | QualifiedIdentifier Arguments
    ;

Selector
    : SeparatorDot Identifier
    | SeparatorDot Identifier Arguments
    | SeparatorDot KeywordThis
    | SeparatorDot NewObjectExpr
    | SeparatorLeftBracket Expression SeparatorRightBracket
    ;

Arguments
    : SeparatorLeftParenthesis ArgumentList SeparatorRightParenthesis
    ;

ArgumentList
    : Expression
    | ArgumentList SeparatorComma Expression
    ;

NewObjectExpr
    : KeywordNew QualifiedIdentifier Arguments
    ;

NewArrayExpr
    : KeywordNew Type DimExprs
    ;

DimExprs
    : DimExpr
    | DimExprs DimExpr
    ;
    
DimExpr
    : SeparatorLeftBracket Expression SeparatorRightBracket
    ;

PostfixOp
    : OperatorIncrement
    | OperatorDecrement
    ;

PrefixOp
    : OperatorIncrement
    | OperatorDecrement
    | OperatorPlus
    | OperatorMinus
    | OperatorBitwiseNot
    | OperatorNot
    ;

InfixOp
    : OperatorPlus
    | OperatorMinus
    | OperatorMultiply
    | OperatorDivide
    | OperatorModulo
    | OperatorXor
    | OperatorBitwiseAnd
    | OperatorBitwiseOr
    | OperatorLessThan
    | OperatorGreaterThan
    | OperatorLessThanOrEqual
    | OperatorGreaterThanOrEqual
    | OperatorEqual
    | OperatorNotEqual
    | OperatorAnd
    | OperatorOr
    ;

/* ========================================================================== */
/*                                  Other rules                               */
/* ========================================================================== */

QualifiedIdentifier
    : Identifier
    | QualifiedIdentifier SeparatorDot Identifier
    ;

Modifier
    : KeywordPublic
    | KeywordProtected
    | KeywordStatic
    | KeywordFinal
    | KeywordAbstract
    | KeywordNative
    ;

ModifiersOpt
    : /* empty */
    | ModifiersOpt Modifier
    ;

Literal
    : IntegerLiteral
    | BooleanLiteral
    | StringLiteral
    | CharacterLiteral
    | NullLiteral
    ;
