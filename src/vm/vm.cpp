#include "../msl_runtime_error.hpp"
#include "constants.hpp"
#include "value_and_heap.hpp"
#include "vm_instr.hpp"
#include <functional>
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
  void pop_void() { --stkptr; }
  Value peek() { return values.at(stkptr - 1); }
  void reserve(uint16_t n) { stkptr += n; }
};

Value vm_buildin_print(LocationRef where, Stack *stack, uint16_t args) {
  if (args != 1) {
    throw msl_runtime_error(where, "expects 1 argument(s)");
  }
  Value val = stack->pop();
  switch (val.tag) {
  case ValueTag::INT:
    println(std::to_string(val.as.INT));
    break;
  case ValueTag::FLOAT:
    println(std::to_string(val.as.FLOAT));
    break;
  case ValueTag::SYMBOL:
    println(resolve_symbol(val.as.SYMBOL));
    break;
  case ValueTag::STRING:
  case ValueTag::LIST:
  case ValueTag::ERROR:
    panic("NOT YET IMPLEMENTED");
  case ValueTag::NONE:
    println("None");
    break;
  }

  return Value::None();
}
Value vm_buildin_panic(LocationRef where, Stack *stack, uint16_t args) {
  panic("NOT YET IMPLEMENTED");
  return Value::None();
}

static std::unordered_map<
    uint64_t, std::function<Value(LocationRef where, Stack *, uint16_t)>>
    vm_fns = {{Constants::BUILDIN_FN_PRINT.index, vm_buildin_print},
              {Constants::BUILDIN_FN_PANIC.index, vm_buildin_panic}

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
      panic("NOT YET IMPLEMENTED");
    case VMTag::VMCALL: {
      Value val = vm_fns.at(instr.as.VMFN.index)(&stack, instr.extra.args);
      stack.push(val);
    } break;
    case VMTag::INIT_FRAME:
    case VMTag::RETURN:
      panic("NOT YET IMPLEMENTED");
    case VMTag::POP: {
      stack.pop_void();
      ++iptr;
    } break;
    case VMTag::PEEK_JMPIF: {
      if (as_bool(instr.where, stack.peek())) {
        iptr = instr.as.INSTRADDR.addr;
      } else {
        ++iptr;
      }
    } break;
    case VMTag::PEEK_JMPIFN: {
      if (!as_bool(instr.where, stack.peek())) {
        iptr = instr.as.INSTRADDR.addr;
      } else {
        ++iptr;
      }
    } break;
    case VMTag::PROGRAM_INIT: {
      stack.reserve(instr.extra.globals);
      ++iptr;
    } break;
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
    case VMTag::JMP: {
      iptr = instr.as.INSTRADDR.addr;
    } break;
    case VMTag::HALT:
      return 0;
      break;
    }
  }
}
