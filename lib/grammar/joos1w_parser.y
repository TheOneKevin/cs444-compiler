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
    | KeywordPackage QualifiedIdentifier ';'
    ;

ImportDeclarations
    : /* empty */
    | ImportDeclarations ImportDeclaration
    ;

ImportDeclaration
    : KeywordImport QualifiedIdentifier ';'
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
    : '{' ClassBodyList '}'
    ;

ClassBodyList
    : /* empty */
    | ClassBodyList ClassBodyDeclaration
    ;

ClassBodyDeclaration
    : ';'
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
    : '{' InterfaceBodyDeclList '}'
    ;

InterfaceBodyDeclList
    : /* empty */
    | InterfaceBodyDeclList InterfaceBodyDeclaration
    ;

InterfaceBodyDeclaration
    : ';'
    | ModifiersOpt InterfaceMemberDecl
    ;

InterfaceMemberDecl
    : Type Identifier FormalParameters ';'
    | KeywordVoid Identifier FormalParameters ';'
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
    | FormalParameters ';'
    ;

ConstructorDeclaratorRest
    : FormalParameters Block
    ;

FormalParameters
    : '(' ')'
    | '(' FormalParameterList ')'
    ;

FormalParameterList
    : FormalParameter
    | FormalParameterList ',' FormalParameter
    ;

FormalParameter
    : Type VariableDeclaratorId
    ;

/* ========================================================================== */
/*                              Block declarations                            */
/* ========================================================================== */

Block
    : '{' BlockStatements '}'
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
    : Type VariableDeclarators ';'
    ;

VariableDeclaratorId
    : Identifier
    | VariableDeclaratorId '[' ']'
    ;

VariableDeclarators
    : VariableDeclarator
    | VariableDeclarators ',' VariableDeclarator
    ;

VariableDeclarator
    : Identifier VariableDeclaratorRest
    ;
    
VariableDeclaratorRest
    : /* empty */
    | '[' '}'
    | '[' '}' OperatorAssign VariableInitializer  
    ;  

VariableInitializer
    : ArrayInitializer
    | Expression
    ;

ArrayInitializer
    : '{' ArrayInitializerList '}'
    ;

ArrayInitializerList
    : Expression
    | ArrayInitializerList ',' Expression
    ;

/* ========================================================================== */
/*                          Statement declarations                            */
/* ========================================================================== */

Statement
    : Block
    | KeywordIf ParExpression Statement
    | KeywordIf ParExpression Statement KeywordElse Statement
    | KeywordFor '('
        LocalVariableDeclarationStatement
        Expression ')'
        Statement
    | KeywordFor '('
        LocalVariableDeclarationStatement
        Expression ';'
        Expression ')'
        Statement
    | KeywordWhile ParExpression Statement
    | KeywordReturn ';'
    | KeywordReturn Expression ';'
    | ';'
    | Expression ';'
    | Identifier ';'
    ;

/* ========================================================================== */
/*                          Type declarations                                 */
/* ========================================================================== */

TypeList
    : Type
    | TypeList ',' Type
    ;

Type
    : QualifiedIdentifier
    | QualifiedIdentifier '[' ']'
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
    | '(' Type ')' Expression3
    | Primary
    | PrimaryNoNewArray Selector
    | PrimaryNoNewArray Selector PostfixOp
    ;

ParExpression
    : '(' Expression ')'
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
    : '.' Identifier
    | '.' Identifier Arguments
    | '.' KeywordThis
    | '.' NewObjectExpr
    | '[' Expression ']'
    ;

Arguments
    : '(' ArgumentList ')'
    ;

ArgumentList
    : Expression
    | ArgumentList ',' Expression
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
    : '[' Expression ']'
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
    | QualifiedIdentifier '.' Identifier
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
