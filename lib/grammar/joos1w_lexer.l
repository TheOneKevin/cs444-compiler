%option noyywrap
%option bison-locations bison-bridge
%option yylineno

%top {
    #include "parser.h"
    #include "parsetree/ParseTreeTypes.h"
    namespace pt = parsetree;
    using plt = parsetree::Literal::Type;
    using pot = parsetree::Operator::Type;
    using pmt = parsetree::Modifier::Type;
    using pbtt = parsetree::BasicType::Type;
}

%{
    int yycolumn = 1;
    #define YY_USER_ACTION \
        yylloc->first_line = yylloc->last_line = yylineno; \
        yylloc->first_column = yycolumn; \
        yylloc->last_column = yycolumn + yyleng - 1; \
        yycolumn += yyleng;

    static void comment(void);
%}

%{
/* ========================================================================== */
/*                          Raw lexer rules (regex)                           */
/* ========================================================================== */
%}

OctalEscape             (\\{ZeroToThree}{OctalDigit}{OctalDigit})|(\\{OctalDigit}{OctalDigit})|(\\{OctalDigit})
OctalDigit              [0-7]
ZeroToThree             [0-3]
EscapeSequence          (\\b|\\t|\\n|\\f|\\r|\\\"|\\\'|\\\\|{OctalEscape})
SingleCharacter         [^'\\]
StringCharacter         [^"\\]|{EscapeSequence}

DecimalIntegerLiteral   (0|[1-9][0-9]*)
BooleanLiteral          (true|false)
CharacterLiteral        \'{SingleCharacter}|{EscapeSequence}\'
StringLiteral           \"{StringCharacter}*\"

Whitespace              [ \t\r\f]+
Newline                 \n
InlineComment           (\/\/.*)|(\/\*\*\/)
MultilineComment        \/(\*|\*\*)
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

"class"         { *yylval = new pt::Poison(); return CLASS; }
"else"          { *yylval = new pt::Poison(); return ELSE; }
"extends"       { *yylval = new pt::Poison(); return EXTENDS; }
"for"           { *yylval = new pt::Poison(); return FOR; }
"if"            { *yylval = new pt::Poison(); return IF; }
"implements"    { *yylval = new pt::Poison(); return IMPLEMENTS; }
"import"        { *yylval = new pt::Poison(); return IMPORT; }
"interface"     { *yylval = new pt::Poison(); return INTERFACE; }
"new"           { *yylval = new pt::Poison(); return NEW; }
"package"       { *yylval = new pt::Poison(); return PACKAGE; }
"return"        { *yylval = new pt::Poison(); return RETURN; }
"void"          { *yylval = new pt::Poison(); return VOID; }
"while"         { *yylval = new pt::Poison(); return WHILE; }
"super"         { *yylval = new pt::Poison(); return SUPER; }

"abstract"  { *yylval = new pt::Modifier(pmt::Abstract);    return ABSTRACT; }
"final"     { *yylval = new pt::Modifier(pmt::Final);       return FINAL; }
"native"    { *yylval = new pt::Modifier(pmt::Native);      return NATIVE; }
"protected" { *yylval = new pt::Modifier(pmt::Protected);   return PROTECTED; }
"public"    { *yylval = new pt::Modifier(pmt::Public);      return PUBLIC; }
"static"    { *yylval = new pt::Modifier(pmt::Static);      return STATIC; }

"boolean"   { *yylval = new pt::BasicType(pbtt::Boolean);   return BOOLEAN; }
"byte"      { *yylval = new pt::BasicType(pbtt::Byte);      return BYTE; }
"char"      { *yylval = new pt::BasicType(pbtt::Char);      return CHAR; }
"short"     { *yylval = new pt::BasicType(pbtt::Short);     return SHORT; }
"int"       { *yylval = new pt::BasicType(pbtt::Int);       return INT; }

"instanceof" {
    *yylval = new pt::Operator(pot::InstanceOf);
    return INSTANCEOF;
}

"this" {
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

{InlineComment} {}

{MultilineComment} {
    comment();
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

"(" { *yylval = new pt::Poison(); return '('; }
")" { *yylval = new pt::Poison(); return ')'; }
"{" { *yylval = new pt::Poison(); return '{'; }
"}" { *yylval = new pt::Poison(); return '}'; }
"[" { *yylval = new pt::Poison(); return '['; }
"]" { *yylval = new pt::Poison(); return ']'; }
";" { *yylval = new pt::Poison(); return ';'; }
"," { *yylval = new pt::Poison(); return ','; }
"." { *yylval = new pt::Poison(); return '.'; }

%{
/* ========================================================================== */
/*                      Operators and Identifiers                             */
/* ========================================================================== */

/* 
Operator: one of 
 	=	>	<	! 
 	==	<=	>=	!=
    &&	||	&   |   
    +	-
    *	/	%   ^
    ~
*/
%}

"="  { *yylval = new pt::Operator(pot::Assign);             return OP_ASSIGN; }
">"  { *yylval = new pt::Operator(pot::GreaterThan);        return OP_GT; }
"<"  { *yylval = new pt::Operator(pot::LessThan);           return OP_LT; }
"!"  { *yylval = new pt::Operator(pot::Not);                return OP_NOT; }
"==" { *yylval = new pt::Operator(pot::Equal);              return OP_EQ; }
"<=" { *yylval = new pt::Operator(pot::LessThanOrEqual);    return OP_LTE; }
">=" { *yylval = new pt::Operator(pot::GreaterThanOrEqual); return OP_GTE; }
"!=" { *yylval = new pt::Operator(pot::NotEqual);           return OP_NEQ; }
"&&" { *yylval = new pt::Operator(pot::And);                return OP_AND; }
"||" { *yylval = new pt::Operator(pot::Or);                 return OP_OR; }
"&"  { *yylval = new pt::Operator(pot::BitwiseAnd);         return OP_BIT_AND; }
"|"  { *yylval = new pt::Operator(pot::BitwiseOr);          return OP_BIT_OR; }
"+"  { *yylval = new pt::Operator(pot::Plus);               return OP_PLUS; }
"-"  { *yylval = new pt::Operator(pot::Minus);              return OP_MINUS; }
"*"  { *yylval = new pt::Operator(pot::Multiply);           return OP_MUL; }
"/"  { *yylval = new pt::Operator(pot::Divide);             return OP_DIV; }
"%"  { *yylval = new pt::Operator(pot::Modulo);             return OP_MOD; }
"^"  { *yylval = new pt::Operator(pot::BitwiseXor);         return OP_BIT_XOR; }
"~"  { *yylval = new pt::Operator(pot::BitwiseNot);         return OP_BIT_NOT; }

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
    (void) lexval;
    (void) location;
    return yylex(&yylval, &yylloc);
}

static void yyerror(const char* s) {
    std::cerr << "Lexing error: " << s << std::endl;
}

static void comment(void) {
    // Iterate through the comment and count the number of newlines
    // so we can update the line number
    int c;
    while ((c = yyinput()) != 0) {
        yycolumn++;
        if (c == '*') {
            while ((c = yyinput()) == '*');
            if (c == '/') return;
            if (c == 0) break;
        }
        if (c == '\n') {
            yylineno++;
            yycolumn = 1;
        }
    }
    yyerror("unterminated comment");
}
