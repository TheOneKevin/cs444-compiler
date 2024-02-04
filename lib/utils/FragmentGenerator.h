#pragma once

#include <regex>

#include "utils/Generator.h"

namespace utils {

class FragmentGenerator {
   using GTy = Generator<std::string>;

public:
   /**
    * @brief Get the next fragment object
    *
    * @param type The type of fragment to generate
    * @return Generator<string> The generator for the fragments
    */
   virtual GTy get_next_fragment(std::string type) = 0;

   /**
    * @brief This will recursively generate the cartesian product of the sets
    * of fragments for each regex group in the input string.
    *
    * @param it    The current regex iterator
    * @param input The current string being built
    * @return Generator<string> A generator for the cartesian product of the
    * fragments
    */
   GTy recur_cart_product(std::sregex_iterator& it, std::string input);

   /**
    * @brief This will find all $<>$ groups in the input string and replace
    * them with the fragments from the fragment generator. The output will be
    * the cartesian product of all the possible fragments.
    *
    * @param input The input string to generate fragments for
    * @return Generator<string> A generator for the cartesian product of the
    * fragments
    */
   GTy match_string(std::string input);
};

} // namespace utils
