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

DecimalIntegerLiteral   [1-9][0-9]*[lL]?
BooleanLiteral          (true|false)
CharacterLiteral        (x)
StringLiteral           (x)

Whitespace              [ \t\r\n\f]+
Comment                 (\/\/[^\n]*[\n])|((\/(\*|\*\*))[.\n]*\*\/)
Identifier              [a-zA-Z_][a-zA-Z0-9_]*

%%

%{
/* ========================================================================== */
/*                               Keywords                                     */
/* ========================================================================== */

/*
Keyword: one of
    abstract    boolean       byte      char        class     const
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
"const"         { return KeywordConst; }
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

"(" { return SeparatorLeftParenthesis; }
")" { return SeparatorRightParenthesis; }
"{" { return SeparatorLeftBrace; }
"}" { return SeparatorRightBrace; }
"[" { return SeparatorLeftBracket; }
"]" { return SeparatorRightBracket; }
";" { return SeparatorSemicolon; }
"," { return SeparatorComma; }
"." { return SeparatorDot; }

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
    *	/	% 
*/
%}

"=" {
    yylval.op = new parsetree::Operator(parsetree::Operator::Type::Assign); 
    return Operator;
}
">" {
    yylval.op = new parsetree::Operator(parsetree::Operator::Type::GreaterThan); 
    return Operator;
}
"<" {
    yylval.op = new parsetree::Operator(parsetree::Operator::Type::LessThan);
    return Operator;
}
"!" {
    yylval.op = new parsetree::Operator(parsetree::Operator::Type::Not);
    return Operator;
}
"==" {
    yylval.op = new parsetree::Operator(parsetree::Operator::Type::Equal);
    return Operator;
}
"<=" {
    yylval.op = new parsetree::Operator(parsetree::Operator::Type::LessThanOrEqual);
    return Operator;
}
">=" {
    yylval.op = new parsetree::Operator(parsetree::Operator::Type::GreaterThanOrEqual);
    return Operator;
}
"!=" {
    yylval.op = new parsetree::Operator(parsetree::Operator::Type::NotEqual);
    return Operator;
}
"&&" {
    yylval.op = new parsetree::Operator(parsetree::Operator::Type::And);
    return Operator;
}
"||" {
    yylval.op = new parsetree::Operator(parsetree::Operator::Type::Or);
    return Operator;
}
"&" {
    yylval.op = new parsetree::Operator(parsetree::Operator::Type::BitwiseAnd);
    return Operator;
}
"|" {
    yylval.op = new parsetree::Operator(parsetree::Operator::Type::BitwiseOr);
    return Operator;
}
"++" {
    yylval.op = new parsetree::Operator(parsetree::Operator::Type::Increment);
    return Operator;
}
"--" {
    yylval.op = new parsetree::Operator(parsetree::Operator::Type::Decrement);
    return Operator;
}
"+" {
    yylval.op = new parsetree::Operator(parsetree::Operator::Type::Plus);
    return Operator;
}
"-" {
    yylval.op = new parsetree::Operator(parsetree::Operator::Type::Minus);
    return Operator;
}
"*" {
    yylval.op = new parsetree::Operator(parsetree::Operator::Type::Multiply);
    return Operator;
}

"/" {
    yylval.op = new parsetree::Operator(parsetree::Operator::Type::Divide);
    return Operator;
}
"%" {
    yylval.op = new parsetree::Operator(parsetree::Operator::Type::Modulo);
    return Operator;
}

%{
/* ========================================================================== */
/*                        Whitespace & comment                                */
/* ========================================================================== */
%}

{Whitespace} { return Whitespace; }

{Comment} { return Comment; }

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
