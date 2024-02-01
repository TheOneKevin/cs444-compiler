#include "tools/gen_fragments/FragmentGenerator.h"

string get_primary_expression(SearchSpace<string> &g) {
    // Our repository of expression fragments
    constexpr char const* fragments[] = {
        "a + b",
        "a && b",
        "a.b.c = 5",
        "this.a.b.c = 5",
        "this/*test*/.a.b.c = a.b.c.d",
        "(int) a.b.c.d",
        "(50) - 50",
        "a + b * c",
        "a == b && c != d",
        "a > b || c < d",
        "a instanceof MyClass",
        "!flag",
        "a.b()",
        "a.b().c()",
        "a.b[1].c.d.[1].e.d.f()[4].e",
        "a = b = c",
        "(f).h()",
        "(h)-g.f()"
    };
    // The length of the fragments array
    constexpr int arr_length = sizeof(fragments) / sizeof(fragments[0]);

    // Check if the generator's requested index is in bounds
    int variant = g.new_parameter();
    if(variant < arr_length) {
        return fragments[variant];
    }

    // Otherwise, reject the parameter and throw
    g.reject(arr_length-1);
    return "";
}

string get_expression(SearchSpace<string> &g) {
    // Based on g, get the fragment
    int variant = g.new_parameter();
    switch(variant) {
        case 0: return get_primary_expression(g);
        case 1: return "a = new a.b.d(" + get_primary_expression(g) + ")";
        case 2: return "(1+2).new Obj(" + get_primary_expression(g) + ")";
        case 3: return "(1+2).new Obj(" + get_primary_expression(g) + ")";
        case 4: return "something.new Obj("
            + get_primary_expression(g) + ").new Obj("
            + get_primary_expression(g) + ").new Obj("
            + get_primary_expression(g) + ")";
        case 5: return "(\"whatthe\").new Obj(" + get_primary_expression(g) + ")";
        case 6: return "('a').new Obj(" + get_primary_expression (g) + ")";
        case 7: return "(999192939).new Obj(" + get_primary_expression (g) + ")";
        case 8: return "(<expr>).new Obj(" + get_primary_expression (g) + ")";
        case 9: return "(int) (" + get_primary_expression (g) + ")";
        case 10: return "(int) new a.b.d.e[" + get_primary_expression (g) + "]";
        case 11: return "a.b.c.d[" + get_primary_expression (g) + "]";
    }
    g.reject(11);
    return "";
}

string get_statement(SearchSpace<string> &g) {
    // Based on g, get the fragment
    string result;
    int variant = g.new_parameter();
    switch(variant) {
        case 1: return ";";
        case 2: return "{}";
        case 3: return "{;}";
        case 4: return "{ { return; } { return; } return; }";
        case 5: return "{{{}{{}{}}{}}{}}}";
        case 6: return "int x = 5;";
        case 7: return "x[] = " + get_primary_expression(g) + ";";
        case 8: return "a.b.c.d. x = " + get_primary_expression(g) + ";";
        case 9: return "int x = " + get_primary_expression(g) + ";";
        case 10: return "return x;";
        case 11: return "return " + get_primary_expression(g) + ";";
        case 12: return "return " + get_expression(g) + ";";
        case 13: return "func();";
        case 14: return "" + get_primary_expression(g) + ";";
        case 15: return "if(" + get_primary_expression(g) + ") if(" 
            + get_primary_expression(g) + ") else";
        case 16: return "if(" + get_primary_expression(g) + ") if(" 
            + get_primary_expression(g) + ") else else";
        case 17: return "if(" + get_primary_expression(g) + ") {} else if (" 
            + get_primary_expression(g) + ") {} else {}";
        case 18: return "while(" + get_primary_expression(g) + ") {}";
        case 19: return "for(;;) {}";
        case 20: return "for(int i = 0; ; i++) {}";
        case 21: return "for(int i = 0; " + get_primary_expression(g) + "; i++) {}";
        case 22: return "for(func(); ; ) {}";
        case 23: return "for(" + get_expression(g) + "; ; ) {}";
        case 24: return "for(func(); ; anotherfunc()) {}";
        case 25: return "for(; ; " + get_expression(g) + ") {}";
    }
    g.reject(25);
    return result;
}
