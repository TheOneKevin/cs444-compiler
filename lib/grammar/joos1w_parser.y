%code top {
    #include <iostream>
    #include "joos1w.parser.tab.h"
    #include "parsetree/ParseTree.h"
    #include "grammar/Joos1WGrammar.h"

    extern int yylex(YYSTYPE*, YYLTYPE*, Joos1WLexer&);
    static void yyerror(YYLTYPE*, YYSTYPE*, Joos1WLexer&, const char*);
}

%code requires {
    #include "parsetree/ParseTree.h"
    namespace pt = parsetree;
    using pty = parsetree::Node::Type;
    class Joos1WLexer;
}

%define api.pure full
%define api.value.type { pt::Node* }
%parse-param { pt::Node** ret }
%param { Joos1WLexer& jl }

%define parse.error verbose
%locations

/* Literal, IDENTIFIER tokens */
%token LITERAL;
%token IDENTIFIER;
%token THIS;

/* Keyword tokens */
%token ABSTRACT BOOLEAN BYTE CHAR CLASS ELSE EXTENDS FINAL FOR IF IMPLEMENTS
%token IMPORT INT INTERFACE NATIVE NEW PACKAGE PROTECTED
%token PUBLIC RETURN SHORT STATIC VOID WHILE

/* Unused tokens */
%token SUPER DOUBLE FLOAT LONG DO TRY CATCH FINALLY THROW THROWS TRANSIENT CASE
%token SYNCHRONIZED VOLATILE CONST GOTO CONTINUE BREAK DEFAULT SWITCH PRIVATE

/* Operator tokens */
%token OP_ASSIGN OP_GT OP_LT OP_NOT OP_EQ OP_LTE OP_GTE OP_NEQ OP_AND
%token OP_OR OP_BIT_AND OP_BIT_OR OP_PLUS OP_MINUS OP_MUL
%token OP_DIV OP_MOD OP_BIT_XOR INSTANCEOF

%start CompilationUnit

%initial-action {
    (void) yynerrs;
    *ret = nullptr;
}

%%

/* ========================================================================== */
/*                          Compliation Unit                                  */
/* ========================================================================== */

CompilationUnit
    : PackageDeclarationOpt ImportDeclarationsOpt TypeDeclarationsOpt {
        *ret = jl.make_node(@$, pty::CompilationUnit, $1, $2, $3);
    }
    ;

PackageDeclarationOpt
    : %empty                                                                    { $$ = nullptr; }
    | PackageDeclaration
    ;

PackageDeclaration
    : PACKAGE QualifiedIdentifier ';'                                           { $$ = jl.make_node(@$, pty::PackageDeclaration, $2); }
    ;

ImportDeclarationsOpt
    : %empty                                                                    { $$ = nullptr; }
    | ImportDeclarationList
    ;

ImportDeclarationList
    : ImportDeclaration                                                         { $$ = jl.make_node(@$, pty::ImportDeclarationList, $1); }
    | ImportDeclarationList ImportDeclaration                                   { $$ = jl.make_node(@$, pty::ImportDeclarationList, $1, $2); }
    ;

ImportDeclaration
    : SingleTypeImportDeclaration
    | TypeImportOnDemandDeclaration
    ;

SingleTypeImportDeclaration
    : IMPORT QualifiedIdentifier ';'                                            { $$ = jl.make_node(@$, pty::SingleTypeImportDeclaration, $2); }
    ;

TypeImportOnDemandDeclaration
    : IMPORT QualifiedIdentifier '.' OP_MUL ';'                                 { $$ = jl.make_node(@$, pty::TypeImportOnDemandDeclaration, $2); }
    ;

TypeDeclarationsOpt
    : %empty                                                                    { $$ = nullptr; }
    | TypeDeclaration
    ;

TypeDeclaration
    : ClassDeclaration
    | InterfaceDeclaration
    ;

/* ========================================================================== */
/*                                 Classes                                    */
/* ========================================================================== */

ClassDeclaration
    : ClassOrInterfaceModifierOpt
      CLASS IDENTIFIER SuperOpt InterfaceOpt ClassBody                          { $$ = jl.make_node(@$, pty::ClassDeclaration, $1, $3, $4, $5, $6 ); }
    ;

ClassOrInterfaceModifierOpt
    : %empty                                                                    { $$ = nullptr; }
    | ClassOrInterfaceModifierList
    ;

ClassOrInterfaceModifierList
    : ClassOrInterfaceModifier                                                  { $$ = jl.make_node(@$, pty::ModifierList, $1); }
    | ClassOrInterfaceModifierList ClassOrInterfaceModifier                     { $$ = jl.make_node(@$, pty::ModifierList, $1, $2); }
    ;

ClassOrInterfaceModifier
    : PUBLIC
    | ABSTRACT
    | FINAL
    ;

SuperOpt
    : %empty                                                                    { $$ = nullptr; }
    | EXTENDS QualifiedIdentifier                                               { $$ = jl.make_node(@$, pty::SuperOpt, $2); }
    ;

InterfaceOpt
    : %empty                                                                    { $$ = nullptr; }
    | IMPLEMENTS InterfaceTypeList                                              { $$ = $2; }
    ;

InterfaceTypeList
    : InterfaceType                                                             { $$ = jl.make_node(@$, pty::InterfaceTypeList, $1); }
    | InterfaceTypeList ',' InterfaceType                                       { $$ = jl.make_node(@$, pty::InterfaceTypeList, $1, $3); }
    ;

InterfaceType
    : QualifiedIdentifier
    ;

ClassBody
    : '{' ClassBodyDeclarationsOpt '}'                                          { $$ = $2; }
    ;

ClassBodyDeclarationsOpt
    : %empty                                                                    { $$ = nullptr; }
    | ClassBodyDeclarationList
    ;

ClassBodyDeclarationList
    : ClassBodyDeclaration                                                      { $$ = jl.make_node(@$, pty::ClassBodyDeclarationList, $1); }
    | ClassBodyDeclarationList ClassBodyDeclaration                             { $$ = jl.make_node(@$, pty::ClassBodyDeclarationList, $1, $2); }
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
    : MemberModifiersOpt Type VariableDeclarator ';'                            { $$ = jl.make_node(@$, pty::FieldDeclaration, $1, $2, $3); }
    ;

MemberModifiersOpt
    : %empty                                                                    { $$ = nullptr; }
    | MemberModifierList
    ;

MemberModifierList
    : MemberModifier                                                            { $$ = jl.make_node(@$, pty::ModifierList, $1); }
    | MemberModifierList MemberModifier                                         { $$ = jl.make_node(@$, pty::ModifierList, $1, $2); }
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
    : MethodHeader MethodBody                                                   { $$ = jl.make_node(@$, pty::MethodDeclaration, $1, $2); }
    ;

MethodHeader
    : MemberModifiersOpt VOID IDENTIFIER '(' FormalParameterListOpt ')'         { $$ = jl.make_node(@$, pty::MethodHeader, $1, $3, $5); }
    | MemberModifiersOpt Type IDENTIFIER '(' FormalParameterListOpt ')'         { $$ = jl.make_node(@$, pty::MethodHeader, $1, $2, $3, $5); }
    ;

FormalParameterListOpt
    : %empty                                                                    { $$ = nullptr; }
    | FormalParameterList
    ;

FormalParameterList
    : FormalParameter                                                           { $$ = jl.make_node(@$, pty::FormalParameterList, $1); }
    | FormalParameterList ',' FormalParameter                                   { $$ = jl.make_node(@$, pty::FormalParameterList, $1, $3); }
    ;

FormalParameter
    : Type IDENTIFIER                                                           { $$ = jl.make_node(@$, pty::FormalParameter, $1, $2); }
    ;

MethodBody
    : Block                                                                     { $$ = $1;}
    | ';'                                                                       { $$ = nullptr; }
    ;

ConstructorDeclaration
    : MemberModifiersOpt
      IDENTIFIER '(' FormalParameterListOpt ')' ConstructorBody                 { $$ = jl.make_node(@$, pty::ConstructorDeclaration, $1, $2, $4, $6); }
    ;

ConstructorBody
    : Block
    ;

/* ========================================================================== */
/*                                Interfaces                                  */
/* ========================================================================== */

InterfaceDeclaration
    : ClassOrInterfaceModifierOpt INTERFACE IDENTIFIER 
        ExtendsInterfacesOpt InterfaceBody                                      { $$ = jl.make_node(@$, pty::InterfaceDeclaration, $1, $3, $4, $5); }
    ;

ExtendsInterfacesOpt
    : %empty                                                                    { $$ = nullptr; }
    | ExtendsInterfaces
    ;

ExtendsInterfaces
    : EXTENDS InterfaceType                                                     { $$ = jl.make_node(@$, pty::InterfaceTypeList, $2); }
    | ExtendsInterfaces ',' InterfaceType                                       { $$ = jl.make_node(@$, pty::InterfaceTypeList, $1, $3); }
    ;

InterfaceBody
    : '{' InterfaceMemberDeclarationsOpt '}'                                    { $$ = $2; }
    ;

InterfaceMemberDeclarationsOpt
    : %empty                                                                    { $$ = nullptr; }
    | InterfaceMemberDeclarationList                                              
    ;

InterfaceMemberDeclarationList
    : AbstractMethodDeclaration                                                 { $$ = jl.make_node(@$, pty::InterfaceMemberDeclarationList, $1); }
    | InterfaceMemberDeclarationList AbstractMethodDeclaration                  { $$ = jl.make_node(@$, pty::InterfaceMemberDeclarationList, $1, $2); }
    ;

AbstractMethodDeclaration
    : AbstractMethodDeclarationOpt
      Type IDENTIFIER '(' FormalParameterListOpt ')' ';'                        { $$ = jl.make_node(@$, pty::AbstractMethodDeclaration, $1, $2, $3, $5); }
    | AbstractMethodDeclarationOpt
      VOID IDENTIFIER '(' FormalParameterListOpt ')' ';'                        { $$ = jl.make_node(@$, pty::AbstractMethodDeclaration, $1, $3, $5); }
    ;

AbstractMethodDeclarationOpt
    : %empty                                                                    { $$ = nullptr; }
    | AbstractMethodModifierList
    ;

AbstractMethodModifierList
    : AbstractMethodModifier                                                    { $$ = jl.make_node(@$, pty::ModifierList, $1); }
    | AbstractMethodModifierList AbstractMethodModifier                         { $$ = jl.make_node(@$, pty::ModifierList, $1, $2); }
    ;

AbstractMethodModifier
    : PUBLIC
    | ABSTRACT
    ;

/* ========================================================================== */
/*             Assignment and Operator Expressions (non-primary)              */
/* ========================================================================== */

ExpressionOpt
    : %empty                                                                    { $$ = nullptr; }
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
    : AssignmentLhsExpression OP_ASSIGN AssignmentExpression                    { $$ = jl.make_node(@$, pty::Expression, $1, $2, $3); }
    ;

AssignmentLhsExpression
    : QualifiedIdentifier
    | FieldAccess
    | ArrayAccess
    ;

PostfixExpression
    : Primary
    | QualifiedIdentifier
    ;

ConditionalOrExpression
    : ConditionalAndExpression
    | ConditionalOrExpression OP_OR ConditionalAndExpression                    { $$ = jl.make_node(@$, pty::Expression, $1, $2, $3); }
    ;

ConditionalAndExpression
    : InclusiveOrExpression
    | ConditionalAndExpression OP_AND InclusiveOrExpression                     { $$ = jl.make_node(@$, pty::Expression, $1, $2, $3); }
    ;

InclusiveOrExpression
    : ExclusiveOrExpression
    | InclusiveOrExpression OP_BIT_OR ExclusiveOrExpression                     { $$ = jl.make_node(@$, pty::Expression, $1, $2, $3); }
    ;

ExclusiveOrExpression
    : AndExpression
    | ExclusiveOrExpression OP_BIT_XOR AndExpression                            { $$ = jl.make_node(@$, pty::Expression, $1, $2, $3); }
    ;

AndExpression
    : EqualityExpression
    | AndExpression OP_BIT_AND EqualityExpression                               { $$ = jl.make_node(@$, pty::Expression, $1, $2, $3); }
    ;

EqualityExpression
    : RelationalExpression
    | EqualityExpression OP_EQ RelationalExpression                             { $$ = jl.make_node(@$, pty::Expression, $1, $2, $3); }
    | EqualityExpression OP_NEQ RelationalExpression                            { $$ = jl.make_node(@$, pty::Expression, $1, $2, $3); }
    ;

RelationalExpression
    : AdditiveExpression
    | RelationalExpression OP_LT AdditiveExpression                             { $$ = jl.make_node(@$, pty::Expression, $1, $2, $3); }
    | RelationalExpression OP_GT AdditiveExpression                             { $$ = jl.make_node(@$, pty::Expression, $1, $2, $3); }
    | RelationalExpression OP_LTE AdditiveExpression                            { $$ = jl.make_node(@$, pty::Expression, $1, $2, $3); }
    | RelationalExpression OP_GTE AdditiveExpression                            { $$ = jl.make_node(@$, pty::Expression, $1, $2, $3); }
    | RelationalExpression INSTANCEOF TypeNotBasic                              { $$ = jl.make_node(@$, pty::Expression, $1, $2, $3); }
    ;

AdditiveExpression
    : MultiplicativeExpression
    | AdditiveExpression OP_PLUS MultiplicativeExpression                       { $$ = jl.make_node(@$, pty::Expression, $1, $2, $3); }
    | AdditiveExpression OP_MINUS MultiplicativeExpression                      { $$ = jl.make_node(@$, pty::Expression, $1, $2, $3); }
    ;

MultiplicativeExpression
    : UnaryExpression
    | MultiplicativeExpression OP_MUL UnaryExpression                           { $$ = jl.make_node(@$, pty::Expression, $1, $2, $3); }
    | MultiplicativeExpression OP_DIV UnaryExpression                           { $$ = jl.make_node(@$, pty::Expression, $1, $2, $3); }
    | MultiplicativeExpression OP_MOD UnaryExpression                           { $$ = jl.make_node(@$, pty::Expression, $1, $2, $3); }
    ;

UnaryExpression
    : OP_MINUS UnaryExpression {
        // If $2 is a Literal, then we mark that literal as negative
        if($2->get_node_type() == pty::Literal) {
            auto* literal = static_cast<pt::Literal*>($2);
            literal->setNegative();
        }
        $$ = jl.make_node(@$, pty::Expression, $1, $2);
    }
    | UnaryExpressionNotPlusMinus
    ;

UnaryExpressionNotPlusMinus
    : PostfixExpression
    | OP_NOT UnaryExpression                                                    { $$ = jl.make_node(@$, pty::Expression, $1, $2); }
    | CastExpression
    ;

CastExpression
    : '(' BasicType Dims ')' UnaryExpression                                    { $$ = jl.make_node(@$, pty::CastExpression, jl.make_node(@2, pty::Type, $2), $3, $5); }
    | '(' Expression ')' UnaryExpressionNotPlusMinus {
        // Cast is valid iff:
        // 1. $2 is a qualified identifier
        // 2. $2 is an array type and has only one child
        bool isType = $2->get_node_type() == pty::QualifiedIdentifier;
        bool isArrType = $2->get_node_type() == pty::ArrayType;
        if(isType) {
            $$ = jl.make_node(@$, pty::CastExpression, jl.make_node(@2, pty::Type, $2), $4);
        } else if (isArrType) {
            $$ = jl.make_node(@$, pty::CastExpression, $2, $4);
        }
        else {
            jl.report_parser_error(@$, "Invalid expression for cast", {@1});
            $$ = jl.make_poison(@$);
        }
    }
    ;

Dims
    : %empty                                                                    { $$ = nullptr; }
    | '[' ']'                                                                   { $$ = jl.make_leaf(@$, pty::Dims); }
    ;

/* ========================================================================== */
/*                          Primary expressions                               */
/* ========================================================================== */

Primary
    : PrimaryNoNewArray
    | ArrayCreationExpression
    | ArrayAccess
    | ArrayCastType
    ;

PrimaryNoNewArray
    : LITERAL
    | THIS
    | '(' Expression ')'                                                        { $$ = jl.make_node(@$, pty::Expression, $2); } /* Needed for literal validation */
    | ClassInstanceCreationExpression
    | FieldAccess
    | MethodInvocation
    ;

FieldAccess
    : Primary '.' IDENTIFIER                                                    { $$ = jl.make_node(@$, pty::FieldAccess, $1, $3); }
    ;

ArrayAccess
    : PrimaryNoNewArray '[' Expression ']'                                      { $$ = jl.make_node(@$, pty::ArrayAccess, $1, $3); }
    | QualifiedIdentifier '[' Expression ']'                                    { $$ = jl.make_node(@$, pty::ArrayAccess, $1, $3); }
    ;

ArrayCastType
    : QualifiedIdentifier '[' ']'                                               { $$ = jl.make_node(@$, pty::ArrayType, $1); }
    ;
MethodInvocation
    : QualifiedIdentifier '(' ArgumentListOpt ')'                               { $$ = jl.make_node(@$, pty::MethodInvocation, $1, $3); }
    | Primary '.' IDENTIFIER '(' ArgumentListOpt ')'                            { $$ = jl.make_node(@$, pty::MethodInvocation, $1, $3, $5); }
    ;

ArrayCreationExpression
    : NEW BasicType '[' Expression ']'                                          { $$ = jl.make_node(@$, pty::ArrayCreationExpression, jl.make_node(@2, pty::ArrayType, $2), $4); }
    | NEW QualifiedIdentifier '[' Expression ']'                                { $$ = jl.make_node(@$, pty::ArrayCreationExpression, jl.make_node(@2, pty::ArrayType, $2), $4); }
    ;

ClassInstanceCreationExpression
    : NEW QualifiedIdentifier '(' ArgumentListOpt ')'                           { $$ = jl.make_node(@$, pty::ClassInstanceCreationExpression, $2, $4); }
    ;

ArgumentListOpt
    : %empty                                                                    { $$ = nullptr; }
    | ArgumentList

ArgumentList
    : Expression                                                                { $$ = jl.make_node(@$, pty::ArgumentList, $1); }
    | ArgumentList ',' Expression                                               { $$ = jl.make_node(@$, pty::ArgumentList, $1, $3); }
    ;

/* ========================================================================== */
/*                   Type, Modifiers and Identifiers                          */
/* ========================================================================== */

Type
    : QualifiedIdentifier                                                       { $$ = jl.make_node(@$, pty::Type, $1); }
    | QualifiedIdentifier '[' ']'                                               { $$ = jl.make_node(@$, pty::ArrayType, $1); }
    | BasicType                                                                 { $$ = jl.make_node(@$, pty::Type, $1); }
    | BasicType '[' ']'                                                         { $$ = jl.make_node(@$, pty::ArrayType, $1); }
    ;

TypeNotBasic
    : QualifiedIdentifier                                                       { $$ = jl.make_node(@$, pty::Type, $1); }
    | QualifiedIdentifier '[' ']'                                               { $$ = jl.make_node(@$, pty::ArrayType, $1); }
    | BasicType '[' ']'                                                         { $$ = jl.make_node(@$, pty::ArrayType, $1); }
    ;

BasicType
    : BOOLEAN
    | BYTE
    | CHAR
    | SHORT
    | INT
    ;

QualifiedIdentifier
    : IDENTIFIER                                                                { $$ = jl.make_node(@$, pty::QualifiedIdentifier, $1); }
    | QualifiedIdentifier '.' IDENTIFIER                                        { $$ = jl.make_node(@$, pty::QualifiedIdentifier, $1, $3); }
    ;

/* ========================================================================== */
/*                          Block and statements                              */
/* ========================================================================== */

Block
    : '{' BlockStatementsOpt '}'                                                { $$ = jl.make_node(@$, pty::Block, $2); }
    ;

BlockStatementsOpt
    : %empty                                                                    { $$ = nullptr; }
    | BlockStatementList
    ;

BlockStatementList
    : BlockStatement                                                            { $$ = jl.make_node(@$, pty::BlockStatementList, $1); }
    | BlockStatementList BlockStatement                                         { $$ = jl.make_node(@$, pty::BlockStatementList, $1, $2); }
    ;

BlockStatement
    : LocalVariableDeclarationStatement                                         /* This is already wrapped */
    | Statement                                                                 /* This is already wrapped */
    ;

/* ========================================================================== */
/*                          Block and statements                              */
/* ========================================================================== */

Statement
    : StatementWithoutTrailingSubstatement                                      /* This is already wrapped */
    | IfThenStatement                                                           { $$ = jl.make_node(@$, pty::Statement, $1); }
	| IfThenElseStatement                                                       { $$ = jl.make_node(@$, pty::Statement, $1); }
	| WhileStatement                                                            { $$ = jl.make_node(@$, pty::Statement, $1); }
	| ForStatement                                                              { $$ = jl.make_node(@$, pty::Statement, $1); }
    ;

StatementWithoutTrailingSubstatement
    : Block                                                                     { $$ = jl.make_node(@$, pty::Statement, $1); }
	| EmptyStatement                                                            /* Empty statement is already wrapped */
    | ExpressionStatement                                                       /* Expression statement is already wrapped */
    | ReturnStatement                                                           { $$ = jl.make_node(@$, pty::Statement, $1); }
    ;

StatementNoShortIf
    : StatementWithoutTrailingSubstatement                                      /* This is already wrapped */
    | IfThenElseStatementNoShortIf                                              { $$ = jl.make_node(@$, pty::Statement, $1); }
    | WhileStatementNoShortIf                                                   { $$ = jl.make_node(@$, pty::Statement, $1); }
    | ForStatementNoShortIf                                                     { $$ = jl.make_node(@$, pty::Statement, $1); }
    ;

ExpressionStatement
    : StatementExpression ';'                                                   { $$ = jl.make_node(@$, pty::Statement, $1); }
    ;

ReturnStatement
    : RETURN ExpressionOpt ';'                                                  { $$ = jl.make_node(@$, pty::ReturnStatement, $2); }
    ;

StatementExpression
    : Assignment                                                                { $$ = jl.make_node(@$, pty::StatementExpression, $1); }
    | MethodInvocation                                                          { $$ = jl.make_node(@$, pty::StatementExpression, $1); }
    | ClassInstanceCreationExpression                                           { $$ = jl.make_node(@$, pty::StatementExpression, $1); }
    ;

EmptyStatement
    : ';'                                                                       { $$ = jl.make_leaf(@$, pty::Statement); }
	;

/* ========================================================================== */
/*                              Control flows                                 */
/* ========================================================================== */

IfThenStatement
    : IF '(' Expression ')' Statement                                           { $$ = jl.make_node(@$, pty::IfThenStatement, $3, $5); }
    ;

IfThenElseStatement
    : IF '(' Expression ')' StatementNoShortIf ELSE Statement                   { $$ = jl.make_node(@$, pty::IfThenStatement, $3, $5, $7); }
    ;

IfThenElseStatementNoShortIf
    : IF '(' Expression ')' StatementNoShortIf ELSE StatementNoShortIf          { $$ = jl.make_node(@$, pty::IfThenStatement, $3, $5, $7); }
    ;

WhileStatement
    : WHILE '(' Expression ')' Statement                                        { $$ = jl.make_node(@$, pty::WhileStatement, $3, $5); }
    ;

WhileStatementNoShortIf
    : WHILE '(' Expression ')' StatementNoShortIf                               { $$ = jl.make_node(@$, pty::WhileStatement, $3, $5); }    
    ;

ForStatement
    : FOR '(' ForInitOpt ';' ExpressionOpt ';' ForUpdateOpt ')' Statement       { $$ = jl.make_node(@$, pty::ForStatement, $3, $5, $7, $9); }
    ;

ForStatementNoShortIf
    : FOR '(' ForInitOpt ';' ExpressionOpt ';' ForUpdateOpt ')' 
        StatementNoShortIf                                                      { $$ = jl.make_node(@$, pty::ForStatement, $3, $5, $7, $9); }
    ;

ForInitOpt
    : %empty                                                                    { $$ = nullptr; }
    | LocalVariableDeclaration                                                  { $$ = jl.make_node(@$, pty::Statement, $1); }
    | StatementExpression                                                       { $$ = jl.make_node(@$, pty::Statement, $1); }
    ;

ForUpdateOpt
    : %empty                                                                    { $$ = nullptr; }
    | StatementExpression                                                       { $$ = jl.make_node(@$, pty::Statement, $1); }
    ;

/* ========================================================================== */
/*                          Variables and Arrays                              */
/* ========================================================================== */

LocalVariableDeclarationStatement
    : LocalVariableDeclaration ';'                                              { $$ = jl.make_node(@$, pty::Statement, $1); }
    ;

LocalVariableDeclaration                                                        
    : Type LocalVariableDeclarator                                              { $$ = jl.make_node(@$, pty::LocalVariableDeclaration, $1, $2); }
    ;

LocalVariableDeclarator
    : IDENTIFIER OP_ASSIGN Expression                                           { $$ = jl.make_node(@$, pty::VariableDeclarator, $1, $3); }

VariableDeclarator
    : IDENTIFIER                                                                { $$ = jl.make_node(@$, pty::VariableDeclarator, $1); }
    | IDENTIFIER OP_ASSIGN Expression                                           { $$ = jl.make_node(@$, pty::VariableDeclarator, $1, $3); }
    ;

%%

std::string joos1w_parser_resolve_token (int yysymbol) {
    if(yysymbol <= 0) return "EOF";
    if(yysymbol >= 256) {
        yysymbol -= 255;
        // Also check if yysymbol is greater than the end of the array
        if((unsigned) yysymbol >= sizeof(yytname)/sizeof(yytname[0])) {
            return std::string{"<unknown>"};
        }
        // Then return the symbol name
        return std::string{
            yysymbol_name(static_cast<yysymbol_kind_t>(yysymbol))
        };
    } else {
        return std::string{static_cast<char>(yysymbol)};
    }
}

static void yyerror(YYLTYPE* yylloc, YYSTYPE* ret, Joos1WLexer& lexer, const char* s) {
    (void) ret;
    lexer.report_parser_error(*yylloc, s);
}
