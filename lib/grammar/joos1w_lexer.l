%option noyywrap
%option bison-locations bison-bridge
%option yylineno

%top {
    #include "parser.h"
    #include "parsetree/ParseTreeTypes.h"
    namespace pt = parsetree;
    using plt = parsetree::Literal::Type;
    using pot = parsetree::Operator::Type;
}

%{
    int yycolumn = 1;
    #define YY_USER_ACTION \
        yylloc->first_line = yylloc->last_line = yylineno; \
        yylloc->first_column = yycolumn; \
        yylloc->last_column = yycolumn + yyleng - 1; \
        yycolumn += yyleng;
%}

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

Whitespace              [ \t\r\f]+
Newline                 \n
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
"void"          { return VOID; }
"while"         { return WHILE; }

"instanceof"    {
    *yylval = new pt::Operator(pot::InstanceOf);
    return INSTANCEOF;
}

"this"          {
    *yylval = new pt::Identifier(yytext);
    return THIS;
}

%{
/* ========================================================================== */
/*                        Whitespace & comment                                */
/* ========================================================================== */
%}

{Whitespace} { }

{Newline} {
    yycolumn = 1;
}

{Comment} {
    // Iterate through the comment and count the number of newlines
    // so we can update the line number
    for (int i = 0; i < yyleng; i++) {
        yycolumn++;
        if (yytext[i] == '\n') {
            yylineno++;
            yycolumn = 1;
        }
    }
    return COMMENT;
}

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
    *yylval = new pt::Literal(plt::Integer, yytext);
    return LITERAL;
}

{BooleanLiteral} {
    *yylval = new pt::Literal(plt::Boolean, yytext); 
    return LITERAL;
}

{CharacterLiteral} {
    *yylval = new pt::Literal(plt::Character, yytext); 
    return LITERAL;
}

{StringLiteral} {
    *yylval = new pt::Literal(plt::String, yytext); 
    return LITERAL;
}

"null" {
    *yylval = new pt::Literal(plt::Null, yytext); 
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

"="  {
    *yylval = new pt::Operator(pot::Assign);
    return OP_ASSIGN;
}
">"  {
    *yylval = new pt::Operator(pot::GreaterThan);
    return OP_GT;
}
"<"  {
    *yylval = new pt::Operator(pot::LessThan);
    return OP_LT;
}
"!"  {
    *yylval = new pt::Operator(pot::Not);
    return OP_NOT;
}
"==" {
    *yylval = new pt::Operator(pot::Equal);
    return OP_EQ;
}
"<=" {
    *yylval = new pt::Operator(pot::LessThanOrEqual);
    return OP_LTE;
}
">=" {
    *yylval = new pt::Operator(pot::GreaterThanOrEqual);
    return OP_GTE;
}
"!=" {
    *yylval = new pt::Operator(pot::NotEqual);
    return OP_NEQ;
}
"&&" {
    *yylval = new pt::Operator(pot::And);
    return OP_AND;
}
"||" {
    *yylval = new pt::Operator(pot::Or);
    return OP_OR;
}
"&"  {
    *yylval = new pt::Operator(pot::BitwiseAnd);
    return OP_BIT_AND;
}
"|"  {
    *yylval = new pt::Operator(pot::BitwiseOr);
    return OP_BIT_OR;
}
"+"  {
    *yylval = new pt::Operator(pot::Plus);
    return OP_PLUS;
}
"-"  {
    *yylval = new pt::Operator(pot::Minus);
    return OP_MINUS;
}
"*"  {
    *yylval = new pt::Operator(pot::Multiply);
    return OP_MUL;
}
"/"  {
    *yylval = new pt::Operator(pot::Divide);
    return OP_DIV;
}
"%"  {
    *yylval = new pt::Operator(pot::Modulo);
    return OP_MOD;
}
"^"  {
    *yylval = new pt::Operator(pot::BitwiseXor);
    return OP_XOR;
}
"~"  {
    *yylval = new pt::Operator(pot::BitwiseNot);
    return OP_BIT_NOT;
}

{Identifier} {
    *yylval = new pt::Identifier(yytext);
    return IDENTIFIER;
}

%{
/* ========================================================================== */
/*                               Unknown                                      */
/* ========================================================================== */
%}

. { return YYUNDEF;}

%%

int yylex(void) {
    // FIXME(kevin): What the fuck is happening here??
    YYSTYPE lexval;
    YYLTYPE location;
    return yylex(&yylval, &yylloc);
}
