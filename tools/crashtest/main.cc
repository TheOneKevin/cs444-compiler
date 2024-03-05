#include <iostream>
#include "utils/Error.h"


void f(int i) {
   if(i >= 42) {
      throw utils::FatalError();
   } else {
      std::cout << "i=" << i << "\n";
      f(i + 1);
   }
}

int main() {
   try {
      f(0);
   } catch(const utils::FatalError &ex) {
      std::cout << ex.what();
   }
}