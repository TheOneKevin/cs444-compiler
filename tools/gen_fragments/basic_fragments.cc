#include "utils/FragmentGenerator.h"
#include "tools/gen_fragments/basic_fragments.h"

using namespace testing;
using std::string;
using utils::Generator;

constexpr auto primary_expr_fragments = {
    "a + b",
    "a && b",
    "a.b.c = 5",
    "this.a.b.c = 5",
    "this/*test*/.a.b.c = a.b.c.d",
    "(int) a.b.c.d",
    "(50) - 50",
    "a + b * c",
    "a == func() && (int) a.b.c != a.b[1].c.d[1].e.d.f(a, b, c, d).g()",
    "a > b || c < d",
    "a instanceof MyClass",
    "!flag",
    "a.b()",
    "a.b().c()",
    "a.b[1].c.d[1].e.d.f(a, b, c, d)",
    "a = b = c",
    "(f).h()",
    "(h)-g.f()",
    "new a.b.c.d.e(a, b, c, d)",
    "new a.b[5]",
//    "array[1+3*2-3/5%6]",
    "a = b = c",
    "a + b",
    "a - b",
    "a * b",
    "a / b",
    "a % b",
    "a == b",
    "a != b",
    "a > b",
    "a < b",
    "a >= b",
    "a <= b",
    "a && b",
    "a || b",
    "!a",
    "a & b",
    "a | b",
    "a ^ b",
    "~a | b ^ a & c & d",
    "(f).h()",
    "(f)-g.h",
    "(int[])-g.h",
    "(f)g.h",
    "((int) f.g)h.i",
//   "this.new a.b.c.d.e()",
//   "arr[5].new a.b.c.d.e()",
//   "'a'.new a()",
//   "new a.b.c.d.e().new a.b.c.d.e()",
//   "new a.b.c.d.e(a, b, c, d).new a.b.c.d.e(a, b, c, d)",
//   "new array[5].new array[5].new Obj()"
};

constexpr auto statement_expression_fragments = {
    "a = b",
    "a.b.c = 5",
    "this.a.b.c = 5",
    "this/*test*/.a.b.c = a.b.c.d",
    "a = b = c",
    "a.b()",
    "a.b().c()",
    "a.b[1].c.d[1].e.d.f(a, b, c, d).g()",
    "(f).h()",
//   "this.new a.b.c.d.e()",
//   "arr[5].new a.b.c.d.e()",
//   "'a'.new a()",
//   "new a.b.c.d.e().new a.b.c.d.e()",
//   "new a.b.c.d.e(a, b, c, d).new a.b.c.d.e(a, b, c, d)",
//   "new array[5].new array[5].new Obj()"
};

constexpr auto expression_fragments = {
    "a = new a.b.d($<pexpr>$)",
    "(1+2).new Obj($<pexpr>$)",
    "(1+2).new Obj($<pexpr>$)",
    "something.new Obj($<pexpr>$).new Obj($<pexpr>$).new Obj($<pexpr>$)",
    "(\"whatthe\").new Obj($<pexpr>$)",
    "('a').new Obj($<pexpr>$)",
    "(999192939).new Obj($<pexpr>$)",
    "($<pexpr>$).new Obj($<pexpr>$)",
    "(int) ($<pexpr>$)",
    "a.b.d.e[$<pexpr>$]",
    "new a.b.d.e[$<pexpr>$]($<pexpr>$, $<pexpr>$)",
    "(int) new a.b.d.e[$<pexpr>$]",
//  ".new a.b.d($<pexpr>$, $<pexpr>$)",
};

constexpr auto statement_fragments = {
    ";",
    "{$<sexpr>$;}",
    "{;}",
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
    "if($<pexpr>$) if($<pexpr>$) {$<sexpr>$;} else {$<sexpr>$;}",
    "if($<pexpr>$) {} else if ($<pexpr>$) {$<sexpr>$;} else {$<sexpr>$;}",
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
