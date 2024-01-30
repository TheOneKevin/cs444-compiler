%option noyywrap

%top {
    #include "parser.h"
    #include "parsetree/ParseTreeTypes.h"
}

%{
/* ========================================================================== */
/*                          Raw lexer rules (regex)                           */
/* ========================================================================== */
%}

JavaCharacter           (\\b|\\t|\\n|\\f|\\r|\\\"|\\\'|\\\\|\\0|.)

DecimalIntegerLiteral   (-?)(0|[1-9][0-9]*)
BooleanLiteral          (true|false)
CharacterLiteral        \'{JavaCharacter}\'
StringLiteral           \"{JavaCharacter}*\"

Whitespace              [ \t\r\n\f]+
Comment                 (\/\/[^\n]*[\n])|((\/(\*|\*\*))(.*\n*)*\*\/)
Identifier              [a-zA-Z_][a-zA-Z0-9_]*

%%

%{
/* ========================================================================== */
/*                               Keywords                                     */
/* ========================================================================== */

/*
Keyword: one of
    abstract    boolean       byte      char        class
    else        extends       final     for         if        implements 
    import      instanceof    int       interface   native    new       
    package     protected     public    return      short     static    
    this        void          while
*/
%}

"abstract"      { return ABSTRACT; }
"boolean"       { return BOOLEAN; }
"byte"          { return BYTE; }
"char"          { return CHAR; }
"class"         { return CLASS; }
"else"          { return ELSE; }
"extends"       { return EXTENDS; }
"final"         { return FINAL; }
"for"           { return FOR; }
"if"            { return IF; }
"implements"    { return IMPLEMENTS; }
"import"        { return IMPORT; }
"instanceof"    { return INSTANCEOF; }
"int"           { return INT; }
"interface"     { return INTERFACE; }
"native"        { return NATIVE; }
"new"           { return NEW; }
"package"       { return PACKAGE; }
"protected"     { return PROTECTED; }
"public"        { return PUBLIC; }
"return"        { return RETURN; }
"short"         { return SHORT; }
"static"        { return STATIC; }
"this"          { return THIS; }
"void"          { return VOID; }
"while"         { return WHILE; }

%{
/* ========================================================================== */
/*                        Whitespace & comment                                */
/* ========================================================================== */
%}

{Whitespace} { }

{Comment} { return COMMENT; }

%{
/* ========================================================================== */
/*                               Literals                                     */
/* ========================================================================== */

/*
Literal: one of
    DecimalIntegerLiteral
    BooleanLiteral
    CharacterLiteral
    StringLiteral
    null
*/
%}

{DecimalIntegerLiteral} {
    yylval.lit = new parsetree::Literal(parsetree::Literal::Type::Integer, yytext);
    return LITERAL;
}

{BooleanLiteral} {
    yylval.lit = new parsetree::Literal(parsetree::Literal::Type::Boolean, yytext); 
    return LITERAL;
}

{CharacterLiteral} {
    yylval.lit = new parsetree::Literal(parsetree::Literal::Type::Character, yytext); 
    return LITERAL;
}

{StringLiteral} {
    yylval.lit = new parsetree::Literal(parsetree::Literal::Type::String, yytext); 
    return LITERAL;
}

"null" {
    yylval.lit = new parsetree::Literal(parsetree::Literal::Type::Null, yytext); 
    return LITERAL;
}

%{
/* ========================================================================== */
/*                               Separators                                   */
/* ========================================================================== */

/*
Separator: one of
	(	)	{	}	[	]	;	,	.
*/
%}

"(" { return '('; }
")" { return ')'; }
"{" { return '{'; }
"}" { return '}'; }
"[" { return '['; }
"]" { return ']'; }
";" { return ';'; }
"," { return ','; }
"." { return '.'; }

%{
/* ========================================================================== */
/*                                 Operators                                  */
/* ========================================================================== */

/* 
Operator: one of 
 	=	>	<	! 
 	==	<=	>=	!=
    &&	||	&   |   
    ++	--  +	-
    *	/	%   ^
    ~
*/
%}

"="  { return OP_ASSIGN; }
">"  { return OP_GT; }
"<"  { return OP_LT; }
"!"  { return OP_NOT; }
"==" { return OP_EQ; }
"<=" { return OP_LTE; }
">=" { return OP_GTE; }
"!=" { return OP_NEQ; }
"&&" { return OP_AND; }
"||" { return OP_OR; }
"&"  { return OP_BIT_AND; }
"|"  { return OP_BIT_OR; }
"++" { return OP_INC; }
"--" { return OP_DEC; }
"+"  { return OP_PLUS; }
"-"  { return OP_MINUS; }
"*"  { return OP_MUL; }
"/"  { return OP_DIV; }
"%"  { return OP_MOD; }
"^"  { return OP_XOR; }
"~"  { return OP_BIT_NOT; }

{Identifier} {
    yylval.id = new parsetree::Identifier(yytext);
    return IDENTIFIER;
}

%{
/* ========================================================================== */
/*                               Unknown                                      */
/* ========================================================================== */
%}

. { return YYUNDEF;}
