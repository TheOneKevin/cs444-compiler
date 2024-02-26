#include "class_fragments.h"

#include "FragmentGenerator.h"

using namespace testing;
using std::string;
using utils::Generator;

// Valid interface combos //////////////////////////////////////////////////////

constexpr auto intf_method_modifiers = {
      "public", "abstract", "" /* default, no modifier */
};

constexpr auto intf_modifiers = {
      "public", "abstract", "final", "" /* default, no modifier */
};

constexpr auto valid_intf_template =
      "$<intf_modifier>$ interface x {"
      "$<intf_method_modifier>$ void y();"
      "}";

// Valid class combos //////////////////////////////////////////////////////////

constexpr auto class_method_modifiers = {
      "public",
      "protected",
      "static",
      "final",
      "abstract",
      "native",
      "" /* default, no modifier */
};

constexpr auto class_member_modifiers = {
      "public",
      "protected",
      "static",
      "final",
      "abstract",
      "" /* default, no modifier */
};

constexpr auto class_modifiers = {
      "public", "final", "abstract", "" /* default, no modifier */
};

constexpr auto valid_class_template =
      "$<class_modifier>$ class x {"
      "$<class_member_modifier>$ int y;"
      "$<class_method_modifier>$ void z() {}"
      "}";

// Generator ///////////////////////////////////////////////////////////////////

Generator<string> ClassGrammarGenerator::get_next_fragment(string type) {
   if(type == "intf_method_modifier") {
      for(auto x : intf_method_modifiers) co_yield x;
   } else if(type == "intf_modifier") {
      for(auto x : intf_modifiers) co_yield x;
   } else if(type == "intf") {
      for(auto x : match_string(valid_intf_template)) co_yield x;
   } else if(type == "class_method_modifier") {
      for(auto x : class_method_modifiers) co_yield x;
   } else if(type == "class_member_modifier") {
      for(auto x : class_member_modifiers) co_yield x;
   } else if(type == "class_modifier") {
      for(auto x : class_modifiers) co_yield x;
   } else if(type == "class") {
      for(auto x : match_string(valid_class_template)) co_yield x;
   }
}
