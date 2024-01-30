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

/* Literal, identifier and comment tokens */
%token<lit> LITERAL;
%token<id>  IDENTIFIER;
%token COMMENT

/* Keyword tokens */
%token ABSTRACT BOOLEAN BYTE CHAR CLASS ELSE EXTENDS FINAL FOR IF IMPLEMENTS
%token IMPORT INSTANCEOF INT INTERFACE NATIVE NEW PACKAGE PROTECTED
%token PUBLIC RETURN SHORT STATIC THIS VOID WHILE

/* Operator tokens */
%token OP_ASSIGN OP_GT OP_LT OP_NOT OP_EQ OP_LTE OP_GTE OP_NEQ OP_AND
%token OP_OR OP_BIT_AND OP_BIT_OR OP_PLUS OP_MINUS OP_MUL OP_DIV OP_MOD
%token OP_XOR OP_BIT_NOT

%start Expression

%%

/* ========================================================================== */
/*            Assignment and Operator Expressions (non-primary)               */
/* ========================================================================== */

Expression
    : AssignmentExpression
    ;

AssignmentExpression
    : Assignment
    | ConditionalOrExpression
    ;

Assignment
    : AssignmentLhsExpression OP_ASSIGN AssignmentExpression
    ;

AssignmentLhsExpression
    : ExpressionName
    | FieldAccess
    | ArrayAccess
    ;

PostfixExpression
    : Primary
    | ExpressionName
    ;

ConditionalOrExpression
    : ConditionalAndExpression
    | ConditionalOrExpression OP_OR ConditionalAndExpression
    ;

ConditionalAndExpression
    : InclusiveOrExpression
    | ConditionalAndExpression OP_AND InclusiveOrExpression
    ;

InclusiveOrExpression
    : ExclusiveOrExpression
    | InclusiveOrExpression OP_BIT_OR ExclusiveOrExpression
    ;

ExclusiveOrExpression
    : AndExpression
    | ExclusiveOrExpression OP_XOR AndExpression
    ;

AndExpression
    : EqualityExpression
    | AndExpression OP_BIT_AND EqualityExpression
    ;

EqualityExpression
    : RelationalExpression
    | EqualityExpression OP_EQ RelationalExpression
    | EqualityExpression OP_NEQ RelationalExpression
    ;

RelationalExpression
    : AdditiveExpression
    | RelationalExpression OP_LT AdditiveExpression
    | RelationalExpression OP_GT AdditiveExpression
    | RelationalExpression OP_LTE AdditiveExpression
    | RelationalExpression OP_GTE AdditiveExpression
    | RelationalExpression INSTANCEOF Type
    ;

AdditiveExpression
    : MultiplicativeExpression
    | AdditiveExpression OP_PLUS MultiplicativeExpression
    | AdditiveExpression OP_MINUS MultiplicativeExpression
    ;

MultiplicativeExpression
    : UnaryExpression
    | MultiplicativeExpression OP_MUL UnaryExpression
    | MultiplicativeExpression OP_DIV UnaryExpression
    | MultiplicativeExpression OP_MOD UnaryExpression
    ;

UnaryExpression
    : PostfixExpression
    | OP_NOT UnaryExpression
    | OP_BIT_NOT UnaryExpression
    | CastExpression
    ;

CastExpression
    : '(' Type ')' UnaryExpression
    ;

/* ========================================================================== */
/*                          Primary expressions                               */
/* ========================================================================== */

Primary
    : PrimaryNoNewArray
    | ArrayCreationExpression
    ;

PrimaryNoNewArray
    : LITERAL
    | THIS
    | '(' Expression ')'
    | ClassInstanceCreationExpression
    | FieldAccess
    | ArrayAccess
    | MethodInvocation
    ;

FieldAccess
    : Primary '.' IDENTIFIER
    ;

ArrayAccess
    : PrimaryNoNewArray '[' Expression ']'
    | ExpressionName '[' Expression ']'
    ;

MethodInvocation
    : IDENTIFIER '(' ArgumentList ')'
    | Primary '.' IDENTIFIER '(' ArgumentList ')'
    ;

ArrayCreationExpression
    : NEW QualifiedIdentifier '[' Expression ']'
    ;

ClassInstanceCreationExpression
    : NEW QualifiedIdentifier '(' ArgumentList ')'
    | Primary '.' NEW QualifiedIdentifier '(' ArgumentList ')'
    ;

ArgumentList
    : Expression
    | ArgumentList ',' Expression
    ;

ExpressionName
    : IDENTIFIER
/*  | QualifiedIdentifier */
    ;

/* ========================================================================== */
/*                   Type, Modifiers and Identifiers                          */
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
    : BOOLEAN
    | BYTE
    | CHAR
    | SHORT
    | INT
    ;

QualifiedIdentifier
    : IDENTIFIER '.' IDENTIFIER
    | QualifiedIdentifier '.' IDENTIFIER
    ;
