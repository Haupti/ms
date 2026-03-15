#include "../lib/asap/util.hpp"
#include "vm/vm.hpp"
#include "parser/parser.hpp"
#include "ir/compile_ir.hpp"
#include "instr/compile_to_vm.hpp"
#include "preprocessor/preprocess.hpp"
#include <filesystem>

using namespace std;
int main(int argc, char **argv) {
  if (argc < 2) {
    fail("expected a entrypoint path as argument");
  }
  filesystem::path path = string(argv[1]);
  std::vector<std::string> msl_args;
  for (int i = 2; i < argc; ++i) {
    msl_args.push_back(argv[i]);
  }
  auto tokens = preprocess(path);
  auto nodes = parse(tokens);
  auto ir = compile_ir(nodes);
  auto instrs = compile_to_vm(ir);
  return run(instrs, msl_args);
}
