#include "utils/FragmentGenerator.h"
#include "class_fragments.h"

using namespace testing;
using std::string;
using utils::Generator;


constexpr auto formal_parameters = {
    "()",
    "(int a, byte b, char c, boolean d, short e)",
    "(int[] a, byte[] b, char[] c, boolean[] d, short[] e)",
    "(Obj a, Obj.d.a. b)"
    "(Obj[] a, Obj.d.a[] b)"
};

constexpr auto statements = {
    ";",
    "{;}",
    "{ { return; } { return; } return; }",
};

// Valid method combos /////////////////////////////////////////////////////////

constexpr auto class_method = {
    "Obj x $<formal_parameters>$ {}",
    "Obj[] x $<formal_parameters>$ {}",
    "<intf_method_modifier> x $<formal_parameters>$ {}",
    "<intf_method_modifier>[] x $<formal_parameters>$ {}",
    "void x $<formal_parameters>$ {}",
    "Obj x $<formal_parameters>$ { $<stmt>$ }",
    "Obj[] x $<formal_parameters>$ { $<stmt>$ }",
    "<intf_method_modifier> x $<formal_parameters>$ { $<stmt>$ }",
    "<intf_method_modifier>[] x $<formal_parameters>$ { $<stmt>$ }",
    "void x $<formal_parameters>$ { $<stmt>$ }",
};

constexpr auto interface_method = {
    "Obj x $<formal_parameters>$ {}",
    "Obj[] x $<formal_parameters>$ {}",
    "$<class_modifier>$ x $<formal_parameters>$ {}",
    "$<class_modifier>$[] x $<formal_parameters>$ {}",
    "void x $<formal_parameters>$ {}",
    "Obj x $<formal_parameters>$ { $<stmt>$ }",
    "Obj[] x $<formal_parameters>$ { $<stmt>$ }",
    "$<class_modifier>$ x $<formal_parameters>$ { $<stmt>$ }",
    "$<class_modifier>$[] x $<formal_parameters>$ { $<stmt>$ }",
    "void x $<formal_parameters>$ { $<stmt>$ }",
};

// Valid method modifier ///////////////////////////////////////////////////////

constexpr auto class_method_modifiers = {
    "public",
    "protected",
    "static",
    "final",
    "abstract",
    "native",
    "" /* default, no modifier */
};

constexpr auto intf_method_modifiers = {
    "public",
    "abstract",
    "" /* default, no modifier */
};

// Generator ///////////////////////////////////////////////////////////////////

Generator<string> MethodGrammarGenerator::get_next_fragment(string type) {  
    if(type == "formal_parameters") {
        for(auto y : match_string(x))
              co_yield y;
    } else if(type == "class_method") {
        for(auto x : match_string(class_method))
            co_yield x;
    } else if(type == "interface_method") {
        for(auto x : match_string(interface_method))
            co_yield x;
    } else if(type == "class_modifier") {
        for(auto x : class_method_modifiers)
            co_yield x;
    } else if(type == "intf_modifier") {
        for(auto x : intf_method_modifiers)
            co_yield x;
    } else if(type == "stmt") {
        for(auto x : statements)
            for(auto y : match_string(x))
                co_yield y;
    }
}
