#include "../lib/asap/util.hpp"
#include "../tests/testutil_token_to_string.hpp"
#include "preprocessor/preprocess.hpp"
#include <filesystem>

using namespace std;
int main(int args, char **argv) {
  if (args != 2) {
    fail("expected a entrypoint path as argument");
  }
  filesystem::path path = string(argv[1]);
  auto tokens = preprocess(path);
  for (auto token : tokens) {
    println(token_to_string(token));
  }
  exit(0);
}
