#include "credits.hpp"
#include <iostream>
void Credits::show_credits() {
  std::cout << "===== CREDITS =====\n"
               "For some of the functionality of this interpreter give credit "
               "to the following software projects.\n"
               "Thank you for providing the code I used to make this project "
               "work!\n";
  std::cout << Credits::STL;
  std::cout << Credits::CLANG;
  std::cout << Credits::PICO_JSON;
  std::cout << Credits::CPP_HTTPLIB;
  std::cout << "===== CREDITS =====\n";
}
