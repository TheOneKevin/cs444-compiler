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
        "new a.b[5]"
        "array[1+3*2-3/5%6]",
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
        "(int[][][])-g.h",
        "(f)g.h",
        "((int) f.g)h.i",
//       "this.new a.b.c.d.e()",
//       "arr[5].new a.b.c.d.e()",
//       "'a'.new a()",
//       "new a.b.c.d.e().new a.b.c.d.e()",
//       "new a.b.c.d.e(a, b, c, d).new a.b.c.d.e(a, b, c, d)",
//       "new array[5].new array[5].new Obj()"
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

// Assignments, MethodInvocation, and ClassInstanceCreationExpression
string get_statement_expression(SearchSpace<string> &g) {
    // Our repository of expression fragments
    constexpr char const* fragments[] = {
        "a = b",
        "a.b.c = 5",
        "this.a.b.c = 5",
        "this/*test*/.a.b.c = a.b.c.d",
        "a = b = c",
        "a.b()",
        "a.b().c()",
        "a.b[1].c.d[1].e.d.f(a, b, c, d).g()",
        "(f).h()"
//       "this.new a.b.c.d.e()",
//       "arr[5].new a.b.c.d.e()",
//       "'a'.new a()",
//       "new a.b.c.d.e().new a.b.c.d.e()",
//       "new a.b.c.d.e(a, b, c, d).new a.b.c.d.e(a, b, c, d)",
//       "new array[5].new array[5].new Obj()"
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
        case 6: return "(" + get_primary_expression (g) + ").new Obj(" + get_primary_expression (g) + ")";
        case 7: return "(" 
            + get_primary_expression (g) + ").new Obj(" 
            + get_primary_expression (g) + ")";
        case 8: return "(int) (" + get_primary_expression (g) + ")";
        case 9: return "(int) new a.b.d.e[" + get_primary_expression (g) + "]";
        case 10: return "a.b.c.d[" + get_primary_expression (g) + "]";
        case 11: return "new a.b.c.d(" 
            + get_primary_expression (g) + ", " 
            + get_primary_expression (g) + ", " 
            + get_primary_expression (g) + ")";
//        case 12: return get_primary_expression (g) + ".new a.b.c.d("
//            + get_primary_expression (g) + ", " 
//            + get_primary_expression (g) + ", "
//            + get_primary_expression (g) + ")";
    }
    g.reject(12);
    return "";
}

string get_statement(SearchSpace<string> &g) {
    // Based on g, get the fragment
    string result;
    int variant = g.new_parameter();
    switch(variant) {
        case 1: return ";";
        case 2: return "{" + get_statement_expression(g) + ";}";
        case 3: return "{;}";
        case 4: return "{ { return; } { return; } return; }";
        case 5: return "{{{}{{}{" + get_statement_expression(g) + ";}}{}}{}}";
        case 6: return "int x = 5;";
        case 7: return "x[] = " + get_primary_expression(g) + ";";
        case 8: return "a.b.c.d. x = " + get_primary_expression(g) + ";";
        case 9: return "int x = " + get_primary_expression(g) + ";";
        case 10: return "return x;";
        case 11: return "return " + get_statement_expression(g) + ";";
        case 12: return "func();";
        case 13: return get_statement_expression(g) + ";";
        case 14: return "if(" + get_primary_expression(g) + ") if(" 
            + get_primary_expression(g) + ") {" 
            + get_statement_expression(g) + ";} else {" 
            + get_statement_expression(g) + ";}";
        case 15: return "if(" + get_primary_expression(g) + ") {} else if (" 
            + get_primary_expression(g) + ") {" 
            + get_statement_expression(g) + ";} else {" 
            + get_statement_expression(g) + ";}";
        case 16: return "while(" + get_primary_expression(g) + ") {" 
            + get_statement_expression(g) + ";}";
        case 17: return "for(;;) {" + get_statement_expression(g) + ";}";
        case 18: return "for(int i = 0; i < func(); i = i + 1) {" 
            + get_statement_expression(g) + ";}";
        case 19: return "for(; " + get_primary_expression(g) + "; ) {" + get_statement_expression(g) + ";}";
        case 20: return "for(" + get_statement_expression(g) + "; ; ) {" 
            + get_statement_expression(g) + ";}";
        case 21: return "for(; ; " + get_statement_expression(g) + ") {" 
            + get_statement_expression(g) + ";}";
    }
    g.reject(21);
    return result;
}
