#include "FragmentGenerator.h"

using namespace utils;
using string = std::string;
using GTy = utils::Generator<string>;

GTy FragmentGenerator::recur_cart_product(std::sregex_iterator& it, string input) {
   const std::sregex_iterator end;
   if(it == end) {
      co_yield input;
   } else {
      std::smatch match = *it;
      string matchedGroup = match[1].str();
      auto next = std::next(it);
      for(auto fragment : get_next_fragment(matchedGroup)) {
         // Create new string with the matched group replaced by the fragment
         string newString = input + match.prefix().str() + fragment;
         // If the next match is the end of the string, append the suffix
         if(next == end) newString += match.suffix().str();
         // Now recursively replace the next regex group
         for(auto x : recur_cart_product(next, newString)) co_yield x;
      }
   }
}

GTy FragmentGenerator::match_string(string input) {
   // Search for the pattern in the input
   static const std::regex pattern{"\\$<([\\w]+)>\\$"};
   std::sregex_iterator it{input.begin(), input.end(), pattern};
   // If there are no matches, return the input string
   // Otherwise, recursively generate the cartesian product
   if(it == std::sregex_iterator{}) {
      co_yield input;
   } else {
      for(auto outputs : recur_cart_product(it, "")) co_yield outputs;
   }
}
