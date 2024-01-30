%parse-param {int *ret}

%code top {
    #include <stdio.h>

    extern int yylex(void);

    static void yyerror(int *ret, const char* s) {
        fprintf(stderr, "%s\n", s);
    }
}

// Tokens and whatnot

%code requires {
    #include "parsetree/ParseTreeTypes.h"
    namespace pt = parsetree;
    using pty = parsetree::Node::Type;
}

%define api.value.type { struct parsetree::Node* }

/* Literal, IDENTIFIER and comment tokens */
%token LITERAL;
%token IDENTIFIER;
%token THIS;
%token COMMENT

/* Keyword tokens */
%token ABSTRACT BOOLEAN BYTE CHAR CLASS ELSE EXTENDS FINAL FOR IF IMPLEMENTS
%token IMPORT INT INTERFACE NATIVE NEW PACKAGE PROTECTED
%token PUBLIC RETURN SHORT STATIC VOID WHILE

/* Operator tokens */
%token OP_ASSIGN OP_GT OP_LT OP_NOT OP_EQ OP_LTE OP_GTE OP_NEQ OP_AND
%token OP_OR OP_BIT_AND OP_BIT_OR OP_PLUS OP_MINUS OP_MUL OP_DIV OP_MOD
%token OP_XOR OP_BIT_NOT INSTANCEOF

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
    : AssignmentLhsExpression OP_ASSIGN AssignmentExpression                    { $$ = new pt::Node(pty::Expression, $1, $3); }
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
    | ConditionalOrExpression OP_OR ConditionalAndExpression                    { $$ = new pt::Node(pty::Expression, $1, $2, $3); }
    ;

ConditionalAndExpression
    : InclusiveOrExpression
    | ConditionalAndExpression OP_AND InclusiveOrExpression                     { $$ = new pt::Node(pty::Expression, $1, $2, $3); }
    ;

InclusiveOrExpression
    : ExclusiveOrExpression
    | InclusiveOrExpression OP_BIT_OR ExclusiveOrExpression                     { $$ = new pt::Node(pty::Expression, $1, $2, $3); }
    ;

ExclusiveOrExpression
    : AndExpression
    | ExclusiveOrExpression OP_XOR AndExpression                                { $$ = new pt::Node(pty::Expression, $1, $2, $3); }
    ;

AndExpression
    : EqualityExpression
    | AndExpression OP_BIT_AND EqualityExpression                               { $$ = new pt::Node(pty::Expression, $1, $2, $3); }
    ;

EqualityExpression
    : RelationalExpression
    | EqualityExpression OP_EQ RelationalExpression                             { $$ = new pt::Node(pty::Expression, $1, $2, $3); }
    | EqualityExpression OP_NEQ RelationalExpression                            { $$ = new pt::Node(pty::Expression, $1, $2, $3); }
    ;

RelationalExpression
    : AdditiveExpression
    | RelationalExpression OP_LT AdditiveExpression                             { $$ = new pt::Node(pty::Expression, $1, $2, $3); }
    | RelationalExpression OP_GT AdditiveExpression                             { $$ = new pt::Node(pty::Expression, $1, $2, $3); }
    | RelationalExpression OP_LTE AdditiveExpression                            { $$ = new pt::Node(pty::Expression, $1, $2, $3); }
    | RelationalExpression OP_GTE AdditiveExpression                            { $$ = new pt::Node(pty::Expression, $1, $2, $3); }
    | RelationalExpression INSTANCEOF Type                                      { $$ = new pt::Node(pty::Expression, $1, $2, $3); }
    ;

AdditiveExpression
    : MultiplicativeExpression
    | AdditiveExpression OP_PLUS MultiplicativeExpression                       { $$ = new pt::Node(pty::Expression, $1, $2, $3); }
    | AdditiveExpression OP_MINUS MultiplicativeExpression                      { $$ = new pt::Node(pty::Expression, $1, $2, $3); }
    ;

MultiplicativeExpression
    : UnaryExpression
    | MultiplicativeExpression OP_MUL UnaryExpression                           { $$ = new pt::Node(pty::Expression, $1, $2, $3); }
    | MultiplicativeExpression OP_DIV UnaryExpression                           { $$ = new pt::Node(pty::Expression, $1, $2, $3); }
    | MultiplicativeExpression OP_MOD UnaryExpression                           { $$ = new pt::Node(pty::Expression, $1, $2, $3); }
    ;

UnaryExpression
    : PostfixExpression
    | OP_NOT UnaryExpression                                                    { $$ = new pt::Node(pty::Expression, $1, $2); }
    | OP_BIT_NOT UnaryExpression                                                { $$ = new pt::Node(pty::Expression, $1, $2); }
    | CastExpression
    ;

CastExpression
    : '(' Type ')' UnaryExpression                                              { $$ = new pt::Node(pty::CastExpression, $1, $2); }
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
    | '(' Expression ')'                                                        { $$ = $2; }
    | ClassInstanceCreationExpression
    | FieldAccess
    | ArrayAccess
    | MethodInvocation
    ;

FieldAccess
    : Primary '.' IDENTIFIER                                                    { $$ = new pt::Node(pty::FieldAccess, $1, $3); }
    ;

ArrayAccess
    : PrimaryNoNewArray '[' Expression ']'                                      { $$ = new pt::Node(pty::ArrayAccess, $1, $3); }
    | ExpressionName '[' Expression ']'                                         { $$ = new pt::Node(pty::ArrayAccess, $1, $3); }
    ;

MethodInvocation
    : IDENTIFIER '(' ArgumentList ')'                                           { $$ = new pt::Node(pty::MethodInvocation, $1, $3); }
    | Primary '.' IDENTIFIER '(' ArgumentList ')'                               { $$ = new pt::Node(pty::MethodInvocation, $1, $3, $5); }
    ;

ArrayCreationExpression
    : NEW QualifiedIdentifier '[' Expression ']'                                { $$ = new pt::Node(pty::ArrayCreationExpression, $2, $4); }
    ;

ClassInstanceCreationExpression
    : NEW QualifiedIdentifier '(' ArgumentList ')'                              { $$ = new pt::Node(pty::ClassInstanceCreationExpression, $2, $4); }
    | Primary '.' NEW QualifiedIdentifier '(' ArgumentList ')'                  { $$ = new pt::Node(pty::ClassInstanceCreationExpression, $1, $3, $5); }
    ;

ArgumentList
    : Expression
    | ArgumentList ',' Expression                                               { $$ = new pt::Node(pty::ArgumentList, $1, $3); }
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
    : QualifiedIdentifier                                                       { $$ = new pt::Node(pty::Type); }
    | QualifiedIdentifier '[' ']'                                               { $$ = new pt::Node(pty::Type); }
    | BasicType                                                                 { $$ = new pt::Node(pty::Type); }
    ;

BasicType
    : BOOLEAN
    | BYTE
    | CHAR
    | SHORT
    | INT
    ;

QualifiedIdentifier
    : IDENTIFIER '.' IDENTIFIER                                                 { $$ = new pt::Node(pty::Identifier); }
    | QualifiedIdentifier '.' IDENTIFIER                                        { $$ = new pt::Node(pty::Identifier); }
    ;
