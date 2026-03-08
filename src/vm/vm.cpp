#include "../msl_runtime_error.hpp"
#include "constants.hpp"
#include "value_and_heap.hpp"
#include "vm_instr.hpp"
#include <vector>
namespace {
// UTILITIES
bool as_bool(LocationRef ref, Value value) {
  switch (value.tag) {
  case ValueTag::SYMBOL:
    return value.as.SYMBOL.index == Constants::SYM_TRUE.index;
  default:
    throw msl_runtime_error(ref, "expected a symbol");
    break;
  }
}

// RUNTIME STUFF

struct Stack {
  std::vector<Value> values;
  uint64_t stkptr;

  Stack(uint64_t init) : stkptr(0) {
    values.reserve(init);

    for (uint64_t i = 0; i < init; ++i) {
      values.push_back(Value());
    }
  }
  void push(const Value &value) {
    values.at(stkptr) = value;
    ++stkptr;
  }
  Value pop() { return values.at(--stkptr); }
};
}; // namespace
int run(std::vector<VMInstr> instrs) {
  VMHeap heap = VMHeap(1000, 1000);
  Stack stack = Stack(10000);
  uint64_t iptr = 0;
  VMInstr instr;
  while (true) {
    instr = instrs.at(iptr);
    switch (instr.tag) {
    case VMTag::PUSH_INT:
      stack.push(Value::Int(instr.as.INT));
      ++iptr;
      break;
    case VMTag::PUSH_FLOAT:
      stack.push(Value::Float(instr.as.FLOAT));
      ++iptr;
      break;
    case VMTag::PUSH_ALLOC_STRING:
      stack.push(heap.add_string(instr.as.STRING));
      ++iptr;
      break;
    case VMTag::PUSH_SYMBOL:
      stack.push(Value::Symbol(instr.as.SYMBOL));
      ++iptr;
      break;
    case VMTag::PUSH_NONE:
      stack.push(Value::None());
      ++iptr;
      break;
    case VMTag::STORE:
    case VMTag::STORE_GLOBAL:
    case VMTag::LOAD:
    case VMTag::LOAD_GLOBAL:
    case VMTag::NOT:
    case VMTag::INVERT:
    case VMTag::ADD:
    case VMTag::SUB:
    case VMTag::MUL:
    case VMTag::DIV:
    case VMTag::MOD:
    case VMTag::LT:
    case VMTag::LTE:
    case VMTag::GT:
    case VMTag::GTE:
    case VMTag::EQ:
    case VMTag::NEQ:
    case VMTag::DUP:
    case VMTag::TYPEOF:
    case VMTag::CALL:
    case VMTag::VMCALL:
    case VMTag::INIT_FRAME:
    case VMTag::RETURN:
    case VMTag::POP:
    case VMTag::ISTRUE:
    case VMTag::ISTRUE_PEEK_JMPIF:
    case VMTag::ISTRUE_PEEK_JMPIFN:
    case VMTag::PROGRAM_INIT:
      panic("NOT YET IMPLEMENTED");
    case VMTag::JMPIFN: {
      Value val = stack.pop();
      if (!as_bool(instr.where, val)) {
        iptr = instr.as.INSTRADDR.addr;
      } else {
        ++iptr;
      }
    } break;
    case VMTag::JMPIF: {
      Value val = stack.pop();
      if (as_bool(instr.where, val)) {
        iptr = instr.as.INSTRADDR.addr;
      } else {
        ++iptr;
      }
    } break;
    case VMTag::JMP:
      iptr = instr.as.INSTRADDR.addr;
      break;
    case VMTag::HALT:
      return 0;
      break;
    }
  }
}
