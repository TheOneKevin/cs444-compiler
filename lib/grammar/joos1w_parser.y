%parse-param {int *ret}

%code top {
    #include <stdio.h>

    extern int yylex(void);

    static void yyerror(int *ret, const char* s) {
        fprintf(stderr, "%s\n", s);
    }
}

// Tokens and whatnot

// yylval is a union of this type

%code requires {
    #include "parsetree/ParseTreeTypes.h"
}

%union {
    struct parsetree::Literal *lit;
    struct parsetree::Operator *op;
    struct parsetree::Identifier *id;
}

%token<lit> IntegerLiteral;
%token<lit> BooleanLiteral;
%token<lit> StringLiteral;
%token<lit> CharacterLiteral;
%token<lit> NullLiteral;
%token<id> Identifier;
%token<op> Operator;
%token SeparatorLeftParenthesis;
%token SeparatorRightParenthesis;
%token SeparatorLeftBrace;
%token SeparatorRightBrace;
%token SeparatorLeftBracket;
%token SeparatorRightBracket;
%token SeparatorSemicolon;
%token SeparatorComma;
%token SeparatorDot;
%token KeywordAbstract;
%token KeywordBoolean;
%token KeywordByte;
%token KeywordChar;
%token KeywordClass;
%token KeywordConst;
%token KeywordElse;
%token KeywordExtends;
%token KeywordFinal;
%token KeywordFor;
%token KeywordIf;
%token KeywordImplements;
%token KeywordImport;
%token KeywordInstanceof;
%token KeywordInt;
%token KeywordInterface;
%token KeywordNative;
%token KeywordNew;
%token KeywordPackage;
%token KeywordProtected;
%token KeywordPublic;
%token KeywordReturn;
%token KeywordShort;
%token KeywordStatic;
%token KeywordThis;
%token KeywordVoid;
%token KeywordWhile;
%token Whitespace;
%token Comment;

// Precedence and associativity

%left PLUS MINUS
%left TIMES
%left UMINUS

%%

// Grammar rules

start:;
