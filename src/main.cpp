#include "../lib/asap/util.hpp"
#include "interpret/interpret.hpp"
#include "parser/parser.hpp"
#include "preprocessor/preprocess.hpp"
#include <filesystem>

using namespace std;
int main(int args, char **argv) {
  if (args != 2) {
    fail("expected a entrypoint path as argument");
  }
  filesystem::path path = string(argv[1]);
  auto tokens = preprocess(path);
  auto nodes = parse(tokens);
  return interpret(nodes);
}
