#pragma once

#include "stack.hpp"
#include "value_and_heap.hpp"
#include "vm_instr.hpp"
#include <vector>

class VM {
public:
  VM(std::vector<VMInstr> instrs, const std::vector<std::string> &msl_args);
  int run();
  bool step();

  // Executes an MSL function reference and returns the result.
  Value call_reference(Value fn_ref, std::vector<Value> args);

  Stack stack;
  VMHeap heap;
  std::vector<VMInstr> instrs;

  uint64_t iptr = 0;
  uint64_t fptr = 0;

private:
  std::vector<uint64_t> ret_stack;
  std::vector<uint64_t> fps_stack;
};

// Keeping the original run for backward compatibility/entrypoint
int run(std::vector<VMInstr> instrs, const std::vector<std::string> &msl_args);
