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

"abstract"      { return KeywordAbstract; }
"boolean"       { return KeywordBoolean; }
"byte"          { return KeywordByte; }
"char"          { return KeywordChar; }
"class"         { return KeywordClass; }
"else"          { return KeywordElse; }
"extends"       { return KeywordExtends; }
"final"         { return KeywordFinal; }
"for"           { return KeywordFor; }
"if"            { return KeywordIf; }
"implements"    { return KeywordImplements; }
"import"        { return KeywordImport; }
"instanceof"    { return KeywordInstanceof; }
"int"           { return KeywordInt; }
"interface"     { return KeywordInterface; }
"native"        { return KeywordNative; }
"new"           { return KeywordNew; }
"package"       { return KeywordPackage; }
"protected"     { return KeywordProtected; }
"public"        { return KeywordPublic; }
"return"        { return KeywordReturn; }
"short"         { return KeywordShort; }
"static"        { return KeywordStatic; }
"this"          { return KeywordThis; }
"void"          { return KeywordVoid; }
"while"         { return KeywordWhile; }

%{
/* ========================================================================== */
/*                        Whitespace & comment                                */
/* ========================================================================== */
%}

{Whitespace} { }

{Comment} { return Comment; }

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
    return IntegerLiteral;
}

{BooleanLiteral} {
    yylval.lit = new parsetree::Literal(parsetree::Literal::Type::Boolean, yytext); 
    return BooleanLiteral;
}

{CharacterLiteral} {
    yylval.lit = new parsetree::Literal(parsetree::Literal::Type::Character, yytext); 
    return CharacterLiteral;
}

{StringLiteral} {
    yylval.lit = new parsetree::Literal(parsetree::Literal::Type::String, yytext); 
    return StringLiteral;
}

"null" {
    yylval.lit = new parsetree::Literal(parsetree::Literal::Type::Null, yytext); 
    return NullLiteral;
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

"="  { return OperatorAssign; }
">"  { return OperatorGreaterThan; }
"<"  { return OperatorLessThan; }
"!"  { return OperatorNot; }
"==" { return OperatorEqual; }
"<=" { return OperatorLessThanOrEqual; }
">=" { return OperatorGreaterThanOrEqual; }
"!=" { return OperatorNotEqual; }
"&&" { return OperatorAnd; }
"||" { return OperatorOr; }
"&"  { return OperatorBitwiseAnd; }
"|"  { return OperatorBitwiseOr; }
"++" { return OperatorIncrement; }
"--" { return OperatorDecrement; }
"+"  { return OperatorPlus; }
"-"  { return OperatorMinus; }
"*"  { return OperatorMultiply; }
"/"  { return OperatorDivide; }
"%"  { return OperatorModulo; }
"^"  { return OperatorXor; }
"~"  { return OperatorBitwiseNot; }

{Identifier} {
    yylval.id = new parsetree::Identifier(yytext);
    return Identifier;
}

%{
/* ========================================================================== */
/*                               Unknown                                      */
/* ========================================================================== */
%}

. { return YYUNDEF;}
