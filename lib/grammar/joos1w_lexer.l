%option noyywrap
%option bison-locations bison-bridge
%option yylineno

%top {
    #include "parser.h"
    #include "parsetree/ParseTreeTypes.h"
    #include <cerrno>
    #include <cstdlib>
    #include <iostream>
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
SingleCharacter         [^\'\\]
StringCharacter         [^"\\]|{EscapeSequence}

DecimalIntegerLiteral   (0|[1-9][0-9]*)
BooleanLiteral          (true|false)
CharacterLiteral        \'({SingleCharacter}|{EscapeSequence})\'
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
    abstract    default    if            private      this
	boolean     do         implements    protected    throw
	break       double     import        public       throws
	byte        else       instanceof    return       transient
	case        extends    int           short        try
	catch       final      interface     static       void
	char        finally    long          strictfp     volatile
	class       float      native        super        while
	const       for        new           switch
	continue    goto       package       synchronized
*/
%}

"class"         { *yylval = pt::make_poison(); return CLASS; }
"else"          { *yylval = pt::make_poison(); return ELSE; }
"extends"       { *yylval = pt::make_poison(); return EXTENDS; }
"for"           { *yylval = pt::make_poison(); return FOR; }
"if"            { *yylval = pt::make_poison(); return IF; }
"implements"    { *yylval = pt::make_poison(); return IMPLEMENTS; }
"import"        { *yylval = pt::make_poison(); return IMPORT; }
"interface"     { *yylval = pt::make_poison(); return INTERFACE; }
"new"           { *yylval = pt::make_poison(); return NEW; }
"package"       { *yylval = pt::make_poison(); return PACKAGE; }
"return"        { *yylval = pt::make_poison(); return RETURN; }
"void"          { *yylval = pt::make_poison(); return VOID; }
"while"         { *yylval = pt::make_poison(); return WHILE; }
"super"         { *yylval = pt::make_poison(); return SUPER; }
"do"            { *yylval = pt::make_poison(); return DO; }
"try"           { *yylval = pt::make_poison(); return TRY; }
"catch"         { *yylval = pt::make_poison(); return CATCH; }
"switch"          { *yylval = pt::make_poison(); return SWITCH; }
"case"          { *yylval = pt::make_poison(); return CASE; }
"default"       { *yylval = pt::make_poison(); return DEFAULT; }

"finally"       { *yylval = pt::make_poison(); return FINALLY; }
"throw"         { *yylval = pt::make_poison(); return THROW; }
"throws"          { *yylval = pt::make_poison(); return THROWS; }
"transient"     { *yylval = pt::make_poison(); return TRANSIENT; }
"synchronized"  { *yylval = pt::make_poison(); return SYNCHRONIZED; }
"volatile"      { *yylval = pt::make_poison(); return VOLATILE; }
"const"         { *yylval = pt::make_poison(); return CONST; }
"goto"          { *yylval = pt::make_poison(); return GOTO; }
"continue"      { *yylval = pt::make_poison(); return CONTINUE; }
"break"         { *yylval = pt::make_poison(); return BREAK; }
"double"        { *yylval = pt::make_poison(); return DOUBLE; }
"float"         { *yylval = pt::make_poison(); return FLOAT; }
"long"          { *yylval = pt::make_poison(); return LONG; }


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
    // Check to see if yytext is in range
    errno = 0;
    char* endptr{};
    const long long int value = std::strtol(yytext, &endptr, 10);
    const long long int INT_MAX = 2147483647ULL;
    if (errno == ERANGE || value > INT_MAX || *endptr != '\0') {
        *yylval = pt::make_poison();
        std::cerr << "Lexing error: integer literal out of range" << std::endl;
        return YYUNDEF;
    }
    // If it is, then we can use it as a literal
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

"(" { *yylval = pt::make_poison(); return '('; }
")" { *yylval = pt::make_poison(); return ')'; }
"{" { *yylval = pt::make_poison(); return '{'; }
"}" { *yylval = pt::make_poison(); return '}'; }
"[" { *yylval = pt::make_poison(); return '['; }
"]" { *yylval = pt::make_poison(); return ']'; }
";" { *yylval = pt::make_poison(); return ';'; }
"," { *yylval = pt::make_poison(); return ','; }
"." { *yylval = pt::make_poison(); return '.'; }

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
