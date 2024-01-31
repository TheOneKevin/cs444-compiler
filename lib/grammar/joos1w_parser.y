%parse-param {int *ret}

%code top {
    #include <stdio.h>
    #include "parsetree/ParseTreeTypes.h"

    extern int yylex(void);

    static void yyerror(int *ret, parsetree::Node** _, const char* s) {
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
%parse-param { struct parsetree::Node** parse_tree_out }

%define parse.error verbose
%locations

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
%token OP_INC OP_DEC OP_OR OP_BIT_AND OP_BIT_OR OP_PLUS OP_MINUS OP_MUL
%token OP_DIV OP_MOD OP_XOR OP_BIT_NOT INSTANCEOF

%start CompilationUnit

%%

/* ========================================================================== */
/*                          Compliation Unit                                  */
/* ========================================================================== */

CompilationUnit
    : PackageDeclarationOpt ImportDeclarationsOpt TypeDeclarationsOpt           { $$ = new pt::Node(pty::CompilationUnit, $1, $2, $3); }               
    ;

PackageDeclarationOpt
    : %empty
    | PackageDeclaration
    ;

PackageDeclaration
    : PACKAGE QualifiedIdentifier ';'                                           { $$ = new pt::Node(pty::PackageDeclaration, $2); }
    ;

ImportDeclarationsOpt
    : %empty
    | ImportDeclarations
    ;

ImportDeclarations
    : ImportDeclaration
    | ImportDeclarations ImportDeclaration                                      { $$ = new pt::Node(pty::ImportDeclarations, $1, $2); }
    ;

ImportDeclaration
    : SingleTypeImportDeclaration
    | TypeImportOnDemandDeclaration
    ;

SingleTypeImportDeclaration
    : IMPORT IDENTIFIER ';'                                                     { $$ = $2; }
    | IMPORT QualifiedIdentifier ';'                                            { $$ = $2; }
    ;

TypeImportOnDemandDeclaration
    : IMPORT IDENTIFIER '.' '*' ';'                                             { $$ = $2; }
    | IMPORT QualifiedIdentifier '.' '*' ';'                                    { $$ = $2; }
    ;

TypeDeclarationsOpt
    : %empty
    | TypeDeclarations
    ;

TypeDeclarations
    : TypeDeclaration
    | TypeDeclarations TypeDeclaration                                          { $$ = new pt::Node(pty::TypeDeclarations, $1, $2); }
    ;

TypeDeclaration
    : ClassDeclaration
    | InterfaceDeclaration
    ;

/* ========================================================================== */
/*                              Classes                                       */
/* ========================================================================== */

ClassDeclaration
    : ClassModifiersOpt CLASS IDENTIFIER SuperOpt InterfaceOpt ClassBody                                    
    ;

ClassModifiersOpt
    : %empty
    | ClassOrInterfaceModifiers
    ;

ClassOrInterfaceModifiers
    : ClassOrInterfaceModifier
    | ClassOrInterfaceModifiers ClassOrInterfaceModifier                        { $$ = new pt::Node(pty::ClassModifiers, $1, $2); }
    ;

ClassOrInterfaceModifier
    : PUBLIC
    | ABSTRACT
    | FINAL
    ;

SuperOpt
    : %empty
    | EXTENDS IDENTIFIER                                                        { $$ = $2; }
    | EXTENDS QualifiedIdentifier                                               { $$ = $2; }
    ;

InterfaceOpt
    : %empty
    | IMPLEMENTS InterfaceTypeList                                              { $$ = $2; }
    ;

InterfaceTypeList
    : InterfaceType
    | InterfaceTypeList ',' InterfaceType                                       { $$ = new pt::Node(pty::InterfaceTypeList, $1, $3); }
    ;

InterfaceType
    : IDENTIFIER
    | QualifiedIdentifier
    ;

ClassBody
    : '{' ClassBodyDeclarationsOpt '}'                                          { $$ = $2; }
    ;

ClassBodyDeclarationsOpt
    : %empty
    | ClassBodyDeclarations
    ;

ClassBodyDeclarations
    : ClassBodyDeclaration
    | ClassBodyDeclarations ClassBodyDeclaration                                { $$ = new pt::Node(pty::ClassBodyDeclarations, $1, $2); }
    ;

ClassBodyDeclaration    
    : ClassMemberDeclaration 
    | ConstructorDeclaration
    ;

ClassMemberDeclaration
    : FieldDeclaration
    | MethodDeclaration
    ;

FieldDeclaration
    : MemberModifiersOpt Type VariableDeclarators ';'                           { $$ = new pt::Node(pty::FieldDeclaration, $1, $2, $3); }
    ;

MemberModifiersOpt
    : %empty
    | MemberModifiers
    ;

MemberModifiers
    : MemberModifier
    | MemberModifiers MemberModifier                                            { $$ = new pt::Node(pty::MemberModifiers, $1, $2); }
    ;

MemberModifier
    : PUBLIC
    | ABSTRACT
    | PROTECTED
    | STATIC
    | FINAL
    | NATIVE
    ;

MethodDeclaration
    : MethodHeader MethodBody                                                   { $$ = new pt::Node(pty::MethodDeclaration, $1, $2); }
    ;

MethodHeader
    : MemberModifiersOpt VOID IDENTIFIER '(' FormalParameterListOpt ')'         { $$ = new pt::Node(pty::MethodDeclaration, $1, $3, $5); }
    | MemberModifiersOpt Type IDENTIFIER '(' FormalParameterListOpt ')'         { $$ = new pt::Node(pty::MethodDeclaration, $1, $2, $3, $5); }
    ;


FormalParameterListOpt
    : %empty
    | FormalParameterList
    ;

FormalParameterList
    : FormalParameter
    | FormalParameterList ',' FormalParameter                                   { $$ = new pt::Node(pty::FormalParameterList, $1, $3); }
    ;

FormalParameter
    : Type IDENTIFIER                                                           { $$ = new pt::Node(pty::FormalParameter, $1, $2); }
    ;

MethodBody
    : Block 
    | ';'
    ;

// Note: Constructors modifiers needs to be checked in the weeder, can only be
//       public or protected

ConstructorDeclaration
    : MemberModifiersOpt IDENTIFIER '(' FormalParameterListOpt ')' 
        ConstructorBody                                                         { $$ = new pt::Node(pty::ConstructorDeclaration, $1, $2, $4, $6); }
    ;

ConstructorBody
    : '{' BlockStatementsOpt '}'                                                { $$ = $2; }
    ;

/* ========================================================================== */
/*                            Interfaces                                      */
/* ========================================================================== */

InterfaceDeclaration
    : ClassOrInterfaceModifiers INTERFACE IDENTIFIER 
        ExtendsInterfacesOpt InterfaceBody                                      { $$ = new pt::Node(pty::InterfaceDeclaration, $1, $3, $4, $5); }
    ;

ExtendsInterfacesOpt
    : %empty
    | ExtendsInterfaces
    ;

ExtendsInterfaces
    : EXTENDS InterfaceType                                                     { $$ = $2; }
    | ExtendsInterfaces ',' InterfaceType                                       { $$ = new pt::Node(pty::ExtendsInterfaces, $1, $3); }
    ;

InterfaceBody
    : '{' InterfaceMemberDeclarationsOpt '}'                                    { $$ = $2; }
    ;

InterfaceMemberDeclarationsOpt
    : %empty
    | InterfaceMemberDeclarations
    ;

InterfaceMemberDeclarations
    : InterfaceMemberDeclaration
    | InterfaceMemberDeclarations InterfaceMemberDeclaration                    { $$ = new pt::Node(pty::InterfaceMemberDeclarations, $1, $2); }
    ;

InterfaceMemberDeclaration
    : AbstractMethodDeclaration
    ;

// Note: Constructors modifiers needs to be checked in the weeder, can only be
//       public or protected

AbstractMethodDeclaration
    : MemberModifiersOpt Type IDENTIFIER '(' FormalParameterListOpt ')' ';'     { $$ = new pt::Node(pty::AbstractMethodDeclaration, $1, $2, $3, $5); }
    | MemberModifiersOpt VOID IDENTIFIER '(' FormalParameterListOpt ')' ';'     { $$ = new pt::Node(pty::AbstractMethodDeclaration, $1, $3, $5); }
    ;

/* ========================================================================== */
/*            Assignment and Operator Expressions (non-primary)               */
/* ========================================================================== */

ExpressionOpt
    : %empty
    | Expression
    ;

Expression
    : AssignmentExpression
    ;

AssignmentExpression
    : Assignment
    | ConditionalOrExpression
    ;

Assignment
    : AssignmentLhsExpression OP_ASSIGN AssignmentExpression                    { $$ = new pt::Node(pty::Expression, $1, $2, $3); }
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
    : '(' Type ')' UnaryExpression                                              { $$ = new pt::Node(pty::CastExpression, $2, $4); }
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
    | Primary '.' NEW QualifiedIdentifier '(' ArgumentList ')'                  { $$ = new pt::Node(pty::ClassInstanceCreationExpression, $1, $4, $6); }
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
    | BasicType '[' ']'                                                         { $$ = new pt::Node(pty::Type); }
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

/* ========================================================================== */
/*                          Block and statements                              */
/* ========================================================================== */

Block
    : '{' BlockStatementsOpt '}'                                                { $$ = new pt::Node(pty::Block, $2); }
    ;

BlockStatementsOpt
    : %empty
    | BlockStatements
    ;

BlockStatements
    : BlockStatement
    | BlockStatements BlockStatement                                            { $$ = new pt::Node(pty::Block, $1, $2); }
    ;

BlockStatement
    : LocalVariableDeclarationStatement
    | Statement
    ;

/* ========================================================================== */
/*                          Block and statements                              */
/* ========================================================================== */

Statement
    : StatementWithoutTrailingSubstatement
    | IfThenStatement
	| IfThenElseStatement
	| WhileStatement
	| ForStatement
    ;

StatementWithoutTrailingSubstatement
    : Block
	| EmptyStatement
    | ExpressionStatement 
    | ReturnStatement
    ;

StatementNoShortIf
    : StatementWithoutTrailingSubstatement
    | IfThenElseStatementNoShortIf
    | WhileStatementNoShortIf
    | ForStatementNoShortIf
    ;

ExpressionStatement
    : StatementExpression ';'                                                   { $$ = $1; }
    ;

ReturnStatement
    : RETURN ExpressionOpt ';'                                                  { $$ = $2; }
    ;

StatementExpression
    : Assignment
    | MethodInvocation
    | ClassInstanceCreationExpression
    ;

EmptyStatement
    : ';'
	;

/* ========================================================================== */
/*                              Control flows                                 */
/* ========================================================================== */

// Note: Expressions for these control flows MUST be of type boolean
//       Though we don't check for that here, need to check in the weeder

IfThenStatement
    : IF '(' Expression ')' Statement                                           { $$ = new pt::Node(pty::IfThenStatement, $3, $5); }
    ;

IfThenElseStatement
    : IF '(' Expression ')' StatementNoShortIf ELSE Statement                   { $$ = new pt::Node(pty::IfThenStatement, $3, $5, $7); }
    ;

IfThenElseStatementNoShortIf
    : IF '(' Expression ')' StatementNoShortIf ELSE StatementNoShortIf          { $$ = new pt::Node(pty::IfThenStatement, $3, $5, $7); }
    ;

WhileStatement
    : WHILE '(' Expression ')' Statement                                        { $$ = new pt::Node(pty::WhileStatement, $3, $5); }
    ;

WhileStatementNoShortIf
    : WHILE '(' Expression ')' StatementNoShortIf                               { $$ = new pt::Node(pty::WhileStatement, $3, $5); }    
    ;

ForStatement
    : FOR '(' ForInit ';' ExpressionOpt ';' ForUpdate ')' Statement             { $$ = new pt::Node(pty::ForStatement, $3, $5, $7, $9); }
    ;

ForStatementNoShortIf
    : FOR '(' ForInit ';' ExpressionOpt ';' ForUpdate ')' StatementNoShortIf    { $$ = new pt::Node(pty::ForStatement, $3, $5, $7, $9); }
    ;

ForInit
    : %empty
    | LocalVariableDeclaration
    | StatementExpression
    ;

ForUpdate
    : %empty
    | StatementExpression
    ;

/* ========================================================================== */
/*                          Variables and Arrays                              */
/* ========================================================================== */

LocalVariableDeclarationStatement
    : LocalVariableDeclaration ';'                                              { $$ = $1; }
    ;

LocalVariableDeclaration                                                        
    : Type VariableDeclarator                                                   { $$ = new pt::Node(pty::LocalVariableDeclaration, $1, $2); }
    ;

VariableDeclarators
    : VariableDeclarator
    | VariableDeclarators ',' VariableDeclarator                                { $$ = new pt::Node(pty::VariableDeclarators, $1, $3); }
    ;

VariableDeclarator
    : IDENTIFIER
    | IDENTIFIER OP_ASSIGN Expression                                           { $$ = $3; }
    ;

%%

std::string joos1w_parser_resolve_token (int yysymbol) {
    if(yysymbol <= 0) return "EOF";
    if(yysymbol >= 256) {
        yysymbol -= 255;
        return std::string{
            yysymbol_name(static_cast<yysymbol_kind_t>(yysymbol))
        };
    } else {
        return std::string{static_cast<char>(yysymbol)};
    }
}
