%code top {
    #include <iostream>
    #include "parsetree/ParseTree.h"

    extern int yylex(void);
    static void yyerror(parsetree::Node** ret, const char* s);
}

%code requires {
    #include "parsetree/ParseTree.h"
    namespace pt = parsetree;
    using pty = parsetree::Node::Type;
}

%define api.value.type { struct parsetree::Node* }
%parse-param { struct parsetree::Node** ret }

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

%%

/* ========================================================================== */
/*                          Compliation Unit                                  */
/* ========================================================================== */

CompilationUnit
    : PackageDeclarationOpt ImportDeclarationsOpt TypeDeclarationsOpt           { *ret = new pt::Node(pty::CompilationUnit, $1, $2, $3); }
    ;

PackageDeclarationOpt
    : %empty                                                                    { $$ = nullptr; }
    | PackageDeclaration
    ;

PackageDeclaration
    : PACKAGE QualifiedIdentifier ';'                                           { $$ = new pt::Node(pty::PackageDeclaration, $2); }
    ;

ImportDeclarationsOpt
    : %empty                                                                    { $$ = nullptr; }
    | ImportDeclarationList
    ;

ImportDeclarationList
    : ImportDeclaration                                                         { $$ = new pt::Node(pty::ImportDeclarationList, $1); }
    | ImportDeclarationList ImportDeclaration                                   { $$ = new pt::Node(pty::ImportDeclarationList, $1, $2); }
    ;

ImportDeclaration
    : SingleTypeImportDeclaration
    | TypeImportOnDemandDeclaration
    ;

SingleTypeImportDeclaration
    : IMPORT QualifiedIdentifier ';'                                            { $$ = new pt::Node(pty::SingleTypeImportDeclaration, $2); }
    ;

TypeImportOnDemandDeclaration
    : IMPORT QualifiedIdentifier '.' OP_MUL ';'                                 { $$ = new pt::Node(pty::TypeImportOnDemandDeclaration, $2); }
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
      CLASS IDENTIFIER SuperOpt InterfaceOpt ClassBody                          { $$ = new pt::Node(pty::ClassDeclaration, $1, $3, $4, $5, $6 ); }
    ;

ClassOrInterfaceModifierOpt
    : %empty                                                                    { $$ = nullptr; }
    | ClassOrInterfaceModifierList
    ;

ClassOrInterfaceModifierList
    : ClassOrInterfaceModifier                                                  { $$ = new pt::Node(pty::ModifierList, $1); }
    | ClassOrInterfaceModifierList ClassOrInterfaceModifier                     { $$ = new pt::Node(pty::ModifierList, $1, $2); }
    ;

ClassOrInterfaceModifier
    : PUBLIC
    | ABSTRACT
    | FINAL
    ;

SuperOpt
    : %empty                                                                    { $$ = nullptr; }
    | EXTENDS QualifiedIdentifier                                               { $$ = new pt::Node(pty::SuperOpt, $2); }
    ;

InterfaceOpt
    : %empty                                                                    { $$ = nullptr; }
    | IMPLEMENTS InterfaceTypeList                                              { $$ = $2; }
    ;

InterfaceTypeList
    : InterfaceType                                                             { $$ = new pt::Node(pty::InterfaceTypeList, $1); }
    | InterfaceTypeList ',' InterfaceType                                       { $$ = new pt::Node(pty::InterfaceTypeList, $1, $3); }
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
    : ClassBodyDeclaration                                                      { $$ = new pt::Node(pty::ClassBodyDeclarationList, $1); }
    | ClassBodyDeclarationList ClassBodyDeclaration                             { $$ = new pt::Node(pty::ClassBodyDeclarationList, $1, $2); }
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
    : MemberModifiersOpt Type VariableDeclaratorList ';'                        { $$ = new pt::Node(pty::FieldDeclaration, $1, $2, $3); }
    ;

MemberModifiersOpt
    : %empty                                                                    { $$ = nullptr; }
    | MemberModifierList
    ;

MemberModifierList
    : MemberModifier                                                            { $$ = new pt::Node(pty::ModifierList, $1); }
    | MemberModifierList MemberModifier                                         { $$ = new pt::Node(pty::ModifierList, $1, $2); }
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
    : MemberModifiersOpt VOID IDENTIFIER '(' FormalParameterListOpt ')'         { $$ = new pt::Node(pty::MethodHeader, $1, $3, $5); }
    | MemberModifiersOpt Type IDENTIFIER '(' FormalParameterListOpt ')'         { $$ = new pt::Node(pty::MethodHeader, $1, $2, $3, $5); }
    ;

FormalParameterListOpt
    : %empty                                                                    { $$ = nullptr; }
    | FormalParameterList
    ;

FormalParameterList
    : FormalParameter                                                           { $$ = new pt::Node(pty::FormalParameterList, $1); }
    | FormalParameterList ',' FormalParameter                                   { $$ = new pt::Node(pty::FormalParameterList, $1, $3); }
    ;

FormalParameter
    : Type IDENTIFIER                                                           { $$ = new pt::Node(pty::FormalParameter, $1, $2); }
    ;

MethodBody
    : Block                                                                     { $$ = $1;}
    | ';'                                                                       { $$ = nullptr; }
    ;

ConstructorDeclaration
    : MemberModifiersOpt
      IDENTIFIER '(' FormalParameterListOpt ')' ConstructorBody                 { $$ = new pt::Node(pty::ConstructorDeclaration, $1, $2, $4, $6); }
    ;

ConstructorBody
    : Block
    ;

/* ========================================================================== */
/*                                Interfaces                                  */
/* ========================================================================== */

InterfaceDeclaration
    : ClassOrInterfaceModifierOpt INTERFACE IDENTIFIER 
        ExtendsInterfacesOpt InterfaceBody                                      { $$ = new pt::Node(pty::InterfaceDeclaration, $1, $3, $4, $5); }
    ;

ExtendsInterfacesOpt
    : %empty                                                                    { $$ = nullptr; }
    | ExtendsInterfaces
    ;

ExtendsInterfaces
    : EXTENDS InterfaceType                                                     { $$ = new pt::Node(pty::InterfaceTypeList, $2); }
    | ExtendsInterfaces ',' InterfaceType                                       { $$ = new pt::Node(pty::InterfaceTypeList, $1, $3); }
    ;

InterfaceBody
    : '{' InterfaceMemberDeclarationsOpt '}'                                    { $$ = $2; }
    ;

InterfaceMemberDeclarationsOpt
    : %empty                                                                    { $$ = nullptr; }
    | InterfaceMemberDeclarationList                                              
    ;

InterfaceMemberDeclarationList
    : AbstractMethodDeclaration                                                 { $$ = new pt::Node(pty::InterfaceMemberDeclarationList, $1); }
    | InterfaceMemberDeclarationList AbstractMethodDeclaration                  { $$ = new pt::Node(pty::InterfaceMemberDeclarationList, $1, $2); }
    ;

AbstractMethodDeclaration
    : AbstractMethodDeclarationOpt
      Type IDENTIFIER '(' FormalParameterListOpt ')' ';'                        { $$ = new pt::Node(pty::AbstractMethodDeclaration, $1, $2, $3, $5); }
    | AbstractMethodDeclarationOpt
      VOID IDENTIFIER '(' FormalParameterListOpt ')' ';'                        { $$ = new pt::Node(pty::AbstractMethodDeclaration, $1, $3, $5); }
    ;

AbstractMethodDeclarationOpt
    : %empty                                                                    { $$ = nullptr; }
    | AbstractMethodModifierList
    ;

AbstractMethodModifierList
    : AbstractMethodModifier                                                    { $$ = new pt::Node(pty::ModifierList, $1); }
    | AbstractMethodModifierList AbstractMethodModifier                         { $$ = new pt::Node(pty::ModifierList, $1, $2); }
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
    : AssignmentLhsExpression OP_ASSIGN AssignmentExpression                    { $$ = new pt::Node(pty::Expression, $1, $2, $3); }
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
    | ExclusiveOrExpression OP_BIT_XOR AndExpression                            { $$ = new pt::Node(pty::Expression, $1, $2, $3); }
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
    | RelationalExpression INSTANCEOF TypeNotBasic                              { $$ = new pt::Node(pty::Expression, $1, $2, $3); }
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
    : OP_MINUS UnaryExpression {
        // FIXME(kevin): Remove this hack and fix the AST later
        // If $2 is a Literal, then we mark that literal as negative :)
        if($2->get_node_type() == pty::Literal) {
            auto* literal = static_cast<pt::Literal*>($2);
            literal->setNegative();
        }
        $$ = new pt::Node(pty::Expression, $1, $2);
    }
    | UnaryExpressionNotPlusMinus
    ;

UnaryExpressionNotPlusMinus
    : PostfixExpression
    | OP_NOT UnaryExpression                                                    { $$ = new pt::Node(pty::Expression, $1, $2); }
    | CastExpression
    ;

CastExpression
    : '(' BasicType Dims ')' UnaryExpression                                    { $$ = new pt::Node(pty::CastExpression, $2, $3, $5); }
    | '(' Expression ')' UnaryExpressionNotPlusMinus {
        // FIXME(kevin): Remove this hack and fix the AST later
        // Cast is valid iff:
        // 1. $2 is a qualified identifier
        // 2. $2 is an array type and has only one child
        bool isType = $2->get_node_type() == pty::QualifiedIdentifier;
        bool isArrType = $2->get_node_type() == pty::ArrayAccess;
        bool hasOneChild = $2->num_children() == 1;
        if(isType || (isArrType && hasOneChild)) {
            $$ = new pt::Node(pty::CastExpression, $2, $4);
        } else {
            std::cerr << "Invalid cast expression" << std::endl;
            $$ = pt::make_poison();
        }
    }
    ;

Dims
    : %empty                                                                    { $$ = nullptr; }
    | '[' ']'                                                                   { $$ = new pt::Node(pty::Dims); }
    ;

/* ========================================================================== */
/*                          Primary expressions                               */
/* ========================================================================== */

Primary
    : PrimaryNoNewArray
    | ArrayCreationExpression
    | ArrayAccess
    ;

PrimaryNoNewArray
    : LITERAL
    | THIS
    | '(' Expression ')'                                                        { $$ = new pt::Node(pty::Expression, $2); } /* Needed for literal validation */
    | ClassInstanceCreationExpression
    | FieldAccess
    | MethodInvocation
    ;

FieldAccess
    : Primary '.' IDENTIFIER                                                    { $$ = new pt::Node(pty::FieldAccess, $1, $3); }
    ;

ArrayAccess
    : PrimaryNoNewArray '[' Expression ']'                                      { $$ = new pt::Node(pty::ArrayAccess, $1, $3); }
    | QualifiedIdentifier '[' Expression ']'                                    { $$ = new pt::Node(pty::ArrayAccess, $1, $3); }
    // FIXME(kevin): This rule is a hack because cast type is actually an expression
    // and needs to be handled in the semantic analysis phase. This rule is to
    // support (x[]) y !!ONLY!! and should be invalid in all other expressions.
    | QualifiedIdentifier '[' ']'                                               { $$ = new pt::Node(pty::ArrayAccess, $1); }
    ;

MethodInvocation
    : QualifiedIdentifier '(' ArgumentListOpt ')'                               { $$ = new pt::Node(pty::MethodInvocation, $1, $3); }
    | Primary '.' IDENTIFIER '(' ArgumentListOpt ')'                            { $$ = new pt::Node(pty::MethodInvocation, $1, $3, $5); }
    ;

ArrayCreationExpression
    : NEW BasicType '[' Expression ']'                                          { $$ = new pt::Node(pty::ArrayCreationExpression, $2, $4); }
    | NEW QualifiedIdentifier '[' Expression ']'                                { $$ = new pt::Node(pty::ArrayCreationExpression, $2, $4); }
    ;

ClassInstanceCreationExpression
    : NEW QualifiedIdentifier '(' ArgumentListOpt ')'                           { $$ = new pt::Node(pty::ClassInstanceCreationExpression, $2, $4); }
    ;

ArgumentListOpt
    : %empty                                                                    { $$ = nullptr; }
    | ArgumentList

ArgumentList
    : Expression                                                                { $$ = new pt::Node(pty::ArgumentList, $1); }
    | ArgumentList ',' Expression                                               { $$ = new pt::Node(pty::ArgumentList, $1, $3); }
    ;

/* ========================================================================== */
/*                   Type, Modifiers and Identifiers                          */
/* ========================================================================== */

Type
    : QualifiedIdentifier                                                       { $$ = new pt::Node(pty::Type, $1); }
    | QualifiedIdentifier '[' ']'                                               { $$ = new pt::Node(pty::ArrayType, $1); }
    | BasicType                                                                 { $$ = new pt::Node(pty::Type, $1); }
    | BasicType '[' ']'                                                         { $$ = new pt::Node(pty::ArrayType, $1); }
    ;

TypeNotBasic
    : QualifiedIdentifier                                                       { $$ = new pt::Node(pty::Type, $1); }
    | QualifiedIdentifier '[' ']'                                               { $$ = new pt::Node(pty::ArrayType, $1); }
    | BasicType '[' ']'                                                         { $$ = new pt::Node(pty::ArrayType, $1); }
    ;

BasicType
    : BOOLEAN
    | BYTE
    | CHAR
    | SHORT
    | INT
    ;

QualifiedIdentifier
    : IDENTIFIER                                                                { $$ = new pt::Node(pty::QualifiedIdentifier, $1); }
    | QualifiedIdentifier '.' IDENTIFIER                                        { $$ = new pt::Node(pty::QualifiedIdentifier, $1, $3); }
    ;

/* ========================================================================== */
/*                          Block and statements                              */
/* ========================================================================== */

Block
    : '{' BlockStatementsOpt '}'                                                { $$ = new pt::Node(pty::Block, $2); }
    ;

BlockStatementsOpt
    : %empty                                                                    { $$ = nullptr; }
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
    : Block                                                                     { $$ = new pt::Node(pty::Statement, $1); }
	| EmptyStatement                                                            /* No action as EmptyStatement returns Statement */
    | ExpressionStatement                                                       /* No action as ExpressionStatement returns Statement */
    | ReturnStatement                                                           /* No action as ReturnStatement returns Statement */
    ;

StatementNoShortIf
    : StatementWithoutTrailingSubstatement
    | IfThenElseStatementNoShortIf
    | WhileStatementNoShortIf
    | ForStatementNoShortIf
    ;

ExpressionStatement
    : StatementExpression ';'                                                   { $$ = new pt::Node(pty::Statement, $1); }
    ;

ReturnStatement
    : RETURN ExpressionOpt ';'                                                  { $$ = new pt::Node(pty::ReturnStatement, $2); }
    ;

StatementExpression
    : Assignment                                                                { $$ = new pt::Node(pty::StatementExpression, $1); }
    | MethodInvocation                                                          { $$ = new pt::Node(pty::StatementExpression, $1); }
    | ClassInstanceCreationExpression                                           { $$ = new pt::Node(pty::StatementExpression, $1); }
    ;

EmptyStatement
    : ';'                                                                       { $$ = new pt::Node(pty::Statement); }
	;

/* ========================================================================== */
/*                              Control flows                                 */
/* ========================================================================== */

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
    : FOR '(' ForInitOpt ';' ExpressionOpt ';' ForUpdateOpt ')' Statement       { $$ = new pt::Node(pty::ForStatement, $3, $5, $7, $9); }
    ;

ForStatementNoShortIf
    : FOR '(' ForInitOpt ';' ExpressionOpt ';' ForUpdateOpt ')' 
        StatementNoShortIf                                                      { $$ = new pt::Node(pty::ForStatement, $3, $5, $7, $9); }
    ;

ForInitOpt
    : %empty                                                                    { $$ = nullptr; }
    | LocalVariableDeclaration                                                  /* No action for union type */
    | StatementExpression                                                       /* No action for union type */
    ;

ForUpdateOpt
    : %empty                                                                    { $$ = nullptr; }
    | StatementExpression                                                       /* No action for optional type */
    ;

/* ========================================================================== */
/*                          Variables and Arrays                              */
/* ========================================================================== */

LocalVariableDeclarationStatement
    : LocalVariableDeclaration ';'                                              { $$ = new pt::Node(pty::Statement, $1); }
    ;

LocalVariableDeclaration                                                        
    : Type VariableDeclarator                                                   { $$ = new pt::Node(pty::LocalVariableDeclaration, $1, $2); }
    ;

// FIXME(kevin): Remove this rule and fix the AST later :)
VariableDeclaratorList
    : VariableDeclarator                                                        { $$ = new pt::Node(pty::VariableDeclaratorList, $1); }
    ;

VariableDeclarator
    : IDENTIFIER                                                                { $$ = new pt::Node(pty::VariableDeclarator, $1); }
    | IDENTIFIER OP_ASSIGN Expression                                           { $$ = new pt::Node(pty::VariableDeclarator, $1, $3); }
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

static void yyerror(parsetree::Node** ret, const char* s) {
    (void) ret;
    // TODO: Grab the location somehow
    std::cerr << "Parse error: " << s << std::endl;
    std::cerr
        << "At: "
        << yylloc.first_line << ":" << yylloc.first_column
        << "-"
        << yylloc.last_line << ":" << yylloc.last_column
        << std::endl;
}
