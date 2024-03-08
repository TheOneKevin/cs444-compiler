%option noyywrap
%option yylineno
%option never-interactive
%option c++ yyclass="Joos1WLexer"

%top {
    #include "joos1w.parser.tab.h"
    #include "parsetree/ParseTree.h"
    #include <cerrno>
    #include <cstdlib>
    #include <iostream>
    using plt = parsetree::Literal::Type;
    using pot = parsetree::Operator::Type;
    using pmt = parsetree::Modifier::Type;
    using pbtt = parsetree::BasicType::Type;
}

%{
    #define INCLUDED_FLEXLEXER_H
    #include "grammar/joos1w_lexer_internal.h"
    #undef INCLUDED_FLEXLEXER_H
    #define YY_USER_ACTION \
        yylloc.first_line = yylloc.last_line = yylineno; \
        yylloc.first_column = yycolumn; \
        yylloc.last_column = yycolumn + yyleng - 1; \
        yycolumn += yyleng;

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

"class"         { yylval = make_poison(yylloc); return CLASS; }
"else"          { yylval = make_poison(yylloc); return ELSE; }
"extends"       { yylval = make_poison(yylloc); return EXTENDS; }
"for"           { yylval = make_poison(yylloc); return FOR; }
"if"            { yylval = make_poison(yylloc); return IF; }
"implements"    { yylval = make_poison(yylloc); return IMPLEMENTS; }
"import"        { yylval = make_poison(yylloc); return IMPORT; }
"interface"     { yylval = make_poison(yylloc); return INTERFACE; }
"new"           { yylval = make_poison(yylloc); return NEW; }
"package"       { yylval = make_poison(yylloc); return PACKAGE; }
"return"        { yylval = make_poison(yylloc); return RETURN; }
"void"          { yylval = make_poison(yylloc); return VOID; }
"while"         { yylval = make_poison(yylloc); return WHILE; }
"super"         { yylval = make_poison(yylloc); return SUPER; }
"do"            { yylval = make_poison(yylloc); return DO; }
"try"           { yylval = make_poison(yylloc); return TRY; }
"catch"         { yylval = make_poison(yylloc); return CATCH; }
"switch"        { yylval = make_poison(yylloc); return SWITCH; }
"case"          { yylval = make_poison(yylloc); return CASE; }
"default"       { yylval = make_poison(yylloc); return DEFAULT; }

"finally"       { yylval = make_poison(yylloc); return FINALLY; }
"throw"         { yylval = make_poison(yylloc); return THROW; }
"throws"        { yylval = make_poison(yylloc); return THROWS; }
"transient"     { yylval = make_poison(yylloc); return TRANSIENT; }
"synchronized"  { yylval = make_poison(yylloc); return SYNCHRONIZED; }
"volatile"      { yylval = make_poison(yylloc); return VOLATILE; }
"const"         { yylval = make_poison(yylloc); return CONST; }
"goto"          { yylval = make_poison(yylloc); return GOTO; }
"continue"      { yylval = make_poison(yylloc); return CONTINUE; }
"break"         { yylval = make_poison(yylloc); return BREAK; }
"double"        { yylval = make_poison(yylloc); return DOUBLE; }
"float"         { yylval = make_poison(yylloc); return FLOAT; }
"long"          { yylval = make_poison(yylloc); return LONG; }
"private"       { yylval = make_poison(yylloc); return PRIVATE; }

"abstract"  { yylval = make_modifier(yylloc, pmt::Abstract);    return ABSTRACT; }
"final"     { yylval = make_modifier(yylloc, pmt::Final);       return FINAL; }
"native"    { yylval = make_modifier(yylloc, pmt::Native);      return NATIVE; }
"protected" { yylval = make_modifier(yylloc, pmt::Protected);   return PROTECTED; }
"public"    { yylval = make_modifier(yylloc, pmt::Public);      return PUBLIC; }
"static"    { yylval = make_modifier(yylloc, pmt::Static);      return STATIC; }

"boolean"   { yylval = make_basic_type(yylloc, pbtt::Boolean);   return BOOLEAN; }
"byte"      { yylval = make_basic_type(yylloc, pbtt::Byte);      return BYTE; }
"char"      { yylval = make_basic_type(yylloc, pbtt::Char);      return CHAR; }
"short"     { yylval = make_basic_type(yylloc, pbtt::Short);     return SHORT; }
"int"       { yylval = make_basic_type(yylloc, pbtt::Int);       return INT; }
"instanceof" {
    yylval = make_operator(yylloc, pot::InstanceOf);
    return INSTANCEOF;
}

"this" {
    yylval = make_identifier(yylloc, yytext);
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
    yylval = make_literal(yylloc, plt::Integer, yytext);
    return LITERAL;
}

{BooleanLiteral} {
    yylval = make_literal(yylloc, plt::Boolean, yytext); 
    return LITERAL;
}

{CharacterLiteral} {
    yylval = make_literal(yylloc, plt::Character, yytext); 
    return LITERAL;
}

{StringLiteral} {
    yylval = make_literal(yylloc, plt::String, yytext); 
    return LITERAL;
}

"null" {
    yylval = make_literal(yylloc, plt::Null, yytext); 
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

"(" { yylval = make_poison(yylloc); return '('; }
")" { yylval = make_poison(yylloc); return ')'; }
"{" { yylval = make_poison(yylloc); return '{'; }
"}" { yylval = make_poison(yylloc); return '}'; }
"[" { yylval = make_poison(yylloc); return '['; }
"]" { yylval = make_poison(yylloc); return ']'; }
";" { yylval = make_poison(yylloc); return ';'; }
"," { yylval = make_poison(yylloc); return ','; }
"." { yylval = make_poison(yylloc); return '.'; }

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

"="  { yylval = make_operator(yylloc, pot::Assign);             return OP_ASSIGN; }
">"  { yylval = make_operator(yylloc, pot::GreaterThan);        return OP_GT; }
"<"  { yylval = make_operator(yylloc, pot::LessThan);           return OP_LT; }
"!"  { yylval = make_operator(yylloc, pot::Not);                return OP_NOT; }
"==" { yylval = make_operator(yylloc, pot::Equal);              return OP_EQ; }
"<=" { yylval = make_operator(yylloc, pot::LessThanOrEqual);    return OP_LTE; }
">=" { yylval = make_operator(yylloc, pot::GreaterThanOrEqual); return OP_GTE; }
"!=" { yylval = make_operator(yylloc, pot::NotEqual);           return OP_NEQ; }
"&&" { yylval = make_operator(yylloc, pot::And);                return OP_AND; }
"||" { yylval = make_operator(yylloc, pot::Or);                 return OP_OR; }
"&"  { yylval = make_operator(yylloc, pot::BitwiseAnd);         return OP_BIT_AND; }
"|"  { yylval = make_operator(yylloc, pot::BitwiseOr);          return OP_BIT_OR; }
"+"  { yylval = make_operator(yylloc, pot::Plus);               return OP_PLUS; }
"-"  { yylval = make_operator(yylloc, pot::Minus);              return OP_MINUS; }
"*"  { yylval = make_operator(yylloc, pot::Multiply);           return OP_MUL; }
"/"  { yylval = make_operator(yylloc, pot::Divide);             return OP_DIV; }
"%"  { yylval = make_operator(yylloc, pot::Modulo);             return OP_MOD; }
"^"  { yylval = make_operator(yylloc, pot::BitwiseXor);         return OP_BIT_XOR; }

{Identifier} {
    yylval = make_identifier(yylloc, yytext);
    return IDENTIFIER;
}

%{
/* ========================================================================== */
/*                               Unknown                                      */
/* ========================================================================== */
%}

. { return YYUNDEF;}

%%

int yylex(YYSTYPE* lvalp, YYLTYPE* llocp, Joos1WLexer& lexer) {
    return lexer.bison_lex(lvalp, llocp);
}

int Joos1WLexer::bison_lex(YYSTYPE *lvalp, YYLTYPE *llocp) {
    auto ret = yylex();
    *lvalp = yylval;
    *llocp = yylloc;
    return ret;
}

void Joos1WLexer::comment(void) {
    // Iterate through the comment and count the number of newlines
    // so we can update the line number
    int c;
    while ((c = yyinput()) != 0) {
        yycolumn++;
        if (c == '\n') {
            yylloc.last_line++;
            yycolumn = 1;
        }
        if (c == '*') {
            while ((c = yyinput()) == '*') { yycolumn++; }
            yycolumn++;
            if (c == '/') return;
            if (c == 0) break;
        }
    }
    auto loc = yylloc;
    loc.first_column = 1;
    loc.first_line = loc.last_line;
    loc.last_column = 1;
    report_parser_error(yylloc, "Unterminated comment");
}
