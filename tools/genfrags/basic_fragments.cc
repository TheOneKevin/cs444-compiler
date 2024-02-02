#include "utils/FragmentGenerator.h"
#include "basic_fragments.h"

using namespace testing;
using std::string;
using utils::Generator;

constexpr auto primary_expr_fragments = {
    "a + b",
    "a.b.c = 5",
    "this",
    "this/*test*/.a.b.c = a.b.c.d",
    "(int) a.b.c.d",
    "(50) - 50",
    "---(10--5+10)/2*-3",
    "a == func() && (int) a.b.c != a.b[1].c.d[1].e.d.f(a, b, c, d).g()",
    "a > b || c < d",
    "a instanceof MyClass",
    "a || a instanceof b",
    "a | a * instanceof b",
    "(Obj) object instanceof cast",
    "!flag",
    "a.b()",
    "a.b().c()",
    "a.b[1].c.d[1].e.d.f(a, b, c, d)",
    "a = b = c",
    "(f).h()",
    "(h)-g.f()",
    "new a.b.c.d.e(a, b, c, d)",
    "new a.b[5]",
    "(new int[1])[0]"
    "(new int[5])[2] = 531"
    "array[1+3*2-3/5%6]",
    "~a | b ^ a & c & !d",
    "(f).h()",
    "(f)-g.h",
    "(int[])-g.h",
    "(f)g.h",
    "((int) f.g)h.i",
    "(int) (a)",
    "(int) new a.b.d.e[a]",
    "(int)(short)(char)(Object)(byte) castMe"
};

constexpr auto statement_expression_fragments = {
    "($<pexpr>$).x = $<pexpr>$",
    "a.b.c = $<pexpr>$",
    "a = new a.b.d($<pexpr>$)",
    "this.a.b.c = 5",
    "this/*test*/.a.b.c = a.b.c.d",
    "a = b = c",
    "a.b()",
    "a.b().c()",
    "a.b[1].c.d[1].e.d.f(a, b, c, d).g()",
    "(f).h()"
};

constexpr auto statement_fragments = {
    ";",
    "{$<sexpr>$;}",
    "{$<sexpr>$;$<sexpr>$;}",
    "{;}",
    "{;;}",
    "{ { return; } { return; } return; }",
    "{{{}{{}{$<sexpr>$;}}{}}{}}",
    "int x = 5;",
    "x[y] = $<pexpr>$;",
    "a.b.c.d. x = $<pexpr>$;",
    "int x = $<pexpr>$;",
    "return x;",
    "return $<sexpr>$;",
    "func();",
    "$<sexpr>$;",
    "if($<pexpr>$) if($<pexpr>$) {} else {$<sexpr>$;}",
    "if($<pexpr>$) {} else if ($<pexpr>$) {$<sexpr>$;} else {}",
    "while($<pexpr>$) {$<sexpr>$;}",
    "for(;;) {$<sexpr>$;}",
    "for(int i = 0; i < func(); i = i + 1) {$<sexpr>$;}",
    "for(; $<pexpr>$; ) {$<sexpr>$;}",
    "for($<sexpr>$; ; ) {$<sexpr>$;}",
    "for(; ; $<sexpr>$) {$<sexpr>$;}",
};

Generator<string> BasicGrammarGenerator::get_next_fragment(string type) {
    if(type == "sexpr") {
        for(auto x : statement_expression_fragments)
            for(auto y : match_string(x))
                co_yield y;
    } else if(type == "pexpr") {
        for(auto x : primary_expr_fragments)
            for(auto y : match_string(x))
                co_yield y;
    } else if(type == "stmt") {
        for(auto x : statement_fragments)
            for(auto y : match_string(x))
                co_yield y;
    }
}
