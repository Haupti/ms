#include "../lib/asap/util.hpp"
#include "instr/compile_to_vm.hpp"
#include "ir/compile_ir.hpp"
#include "parser/parser.hpp"
#include "preprocessor/preprocess.hpp"
#include "vm/vm.hpp"
#include <iostream>
#include <string>

void dump_vm_instructions(const std::vector<VMInstr> &instrs) {
  for (uint64_t i = 0; i < instrs.size(); ++i) {
    std::cout << vminstr_to_string(i, instrs[i]) << std::endl;
  }
}

using namespace std;
int main(int argc, char **argv) {
  bool dump_vm = false;
  string entrypoint = "";
  std::vector<std::string> msl_args;

  for (int i = 1; i < argc; ++i) {
    string arg = argv[i];
    if (arg == "--dump-vm") {
      dump_vm = true;
    } else if (entrypoint == "") {
      entrypoint = arg;
    } else {
      msl_args.push_back(arg);
    }
  }

  if (entrypoint == "") {
    fail("expected a entrypoint path as argument");
  }

  string path = entrypoint;
  auto tokens = preprocess(path, true);
  auto nodes = parse(tokens);
  auto ir = compile_ir(nodes);
  auto instrs = compile_to_vm(ir);

  if (dump_vm) {
    dump_vm_instructions(instrs);
    return 0;
  }

  return run(instrs, msl_args);
}
