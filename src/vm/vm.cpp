#include "../msl_runtime_error.hpp"
#include "constants.hpp"
#include "value_and_heap.hpp"
#include "vm_instr.hpp"
#include <functional>
#include <stack>
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
  Value pop() {
    Value val = values.at(--stkptr);
    if (val.undefined) {
      panic("UNEXPECTED UNDEFINED VALUE (this is a bug in the VM)");
    }
    return val;
  }
  void pop_void() { --stkptr; }
  Value peek() {
    Value val = values.at(stkptr - 1);
    if (val.undefined) {
      panic("UNEXPECTED UNDEFINED VALUE (this is a bug in the VM)");
    }
    return val;
  }
  void reserve(uint16_t n) { stkptr += n; }
  void write_local(uint64_t fptr, StkAddr addr, Value val) {
    values.at(fptr + addr.addr) = val;
  }
  void write_global(StkAddr addr, Value val) { values.at(addr.addr) = val; }
  Value load_global(StkAddr addr) { return values.at(addr.addr); }
  Value load_local(uint64_t fptr, StkAddr addr) {
    return values.at(fptr + addr.addr);
  }
  void dup() {
    Value val = values.at(stkptr - 1);
    push(val);
  }
  void allocate(uint16_t locals) {
    uint64_t end = stkptr + locals;
    uint64_t curr = stkptr;
    for (uint64_t i = curr; i < end; ++i) {
      values.at(i).undefined = true;
      ++stkptr;
    }
  }
  void retreat(uint16_t frame_start) {
    uint64_t curr = stkptr;
    for (uint64_t i = curr; i > frame_start; --i) {
      --stkptr;
      values.at(i).undefined = true;
    }
  }
};

std::string value_to_string(Stack *stack, VMHeap *heap, Value value) {
  switch (value.tag) {
  case ValueTag::INT:
    return std::to_string(value.as.INT);
  case ValueTag::FLOAT:
    return std::to_string(value.as.FLOAT);
  case ValueTag::SYMBOL:
    return resolve_symbol(value.as.SYMBOL);
  case ValueTag::STRING:
    return heap->get_string(value.as.STRING);
  case ValueTag::LIST:
    panic("NOT YET IMPLEMENTED");
  case ValueTag::ERROR:
    return "error(" + value_to_string(stack, heap, heap->at(value.as.ERROR)) +
           ")";
  case ValueTag::NONE:
    return "None";
  }
}

bool values_equal(VMHeap *heap, Value left, Value right) {
  switch (left.tag) {
  case ValueTag::INT:
    switch (right.tag) {
    case ValueTag::INT:
      return left.as.INT == right.as.INT;
    case ValueTag::FLOAT:
      return left.as.INT == right.as.FLOAT;
    default:
      return false;
    }
  case ValueTag::FLOAT:
    switch (right.tag) {
    case ValueTag::INT:
      return left.as.FLOAT == right.as.INT;
    case ValueTag::FLOAT:
      return left.as.FLOAT == right.as.FLOAT;
    default:
      return false;
    }
  case ValueTag::SYMBOL:
    return right.tag == ValueTag::SYMBOL &&
           left.as.SYMBOL.index == right.as.SYMBOL.index;
  case ValueTag::STRING:
    return right.tag == ValueTag::STRING &&
           (left.as.STRING == right.as.STRING ||
            (heap->get_string(left.as.STRING) ==
             heap->get_string(right.as.STRING)));
  case ValueTag::LIST:
    return right.tag == ValueTag::LIST && left.as.LIST == right.as.LIST;
  case ValueTag::ERROR:
    return right.tag == ValueTag::ERROR &&
           values_equal(heap, heap->at(left.as.ERROR),
                        heap->at(right.as.ERROR));
  case ValueTag::NONE:
    return right.tag == ValueTag::NONE;
  }
}

Value vm_buildin_print(LocationRef where, Stack *stack, VMHeap *heap,
                       uint16_t args) {
  if (args != 1) {
    throw msl_runtime_error(where, "expects 1 argument(s)");
  }
  println(value_to_string(stack, heap, stack->pop()));
  return Value::None();
}
[[noreturn]] Value vm_buildin_panic(LocationRef where, Stack *stack,
                                    VMHeap *heap, uint16_t args) {
  if (args != 1) {
    throw msl_runtime_error(where, "expects 1 argument(s)");
  }
  throw msl_runtime_error(where, value_to_string(stack, heap, stack->pop()));
}

static std::unordered_map<
    uint64_t,
    std::function<Value(LocationRef where, Stack *, VMHeap *, uint16_t)>>
    vm_fns = {{Constants::BUILDIN_FN_PRINT.index, vm_buildin_print},
              {Constants::BUILDIN_FN_PANIC.index, vm_buildin_panic}

};

void vm_add(LocationRef where, Stack *stack) {
  Value right = stack->pop();
  Value left = stack->pop();
  switch (left.tag) {
  case ValueTag::INT:
    switch (right.tag) {
    case ValueTag::INT:
      stack->push(Value::Int(left.as.INT + right.as.INT));
      break;
    case ValueTag::FLOAT:
      stack->push(Value::Float(left.as.INT + right.as.FLOAT));
      break;
    default:
      throw msl_runtime_error(where, "expected a number as right argument");
    }
  case ValueTag::FLOAT:
    switch (right.tag) {
    case ValueTag::INT:
      stack->push(Value::Float(left.as.FLOAT + right.as.INT));
      break;
    case ValueTag::FLOAT:
      stack->push(Value::Float(left.as.FLOAT + right.as.FLOAT));
      break;
    default:
      throw msl_runtime_error(where, "expected a number as right argument");
    }
  default:
    throw msl_runtime_error(where, "expected a number as left argument");
  }
}
void vm_sub(LocationRef where, Stack *stack) {
  Value right = stack->pop();
  Value left = stack->pop();
  switch (left.tag) {
  case ValueTag::INT:
    switch (right.tag) {
    case ValueTag::INT:
      stack->push(Value::Int(left.as.INT - right.as.INT));
      break;
    case ValueTag::FLOAT:
      stack->push(Value::Float(left.as.INT - right.as.FLOAT));
      break;
    default:
      throw msl_runtime_error(where, "expected a number as right argument");
    }
  case ValueTag::FLOAT:
    switch (right.tag) {
    case ValueTag::INT:
      stack->push(Value::Float(left.as.FLOAT - right.as.INT));
      break;
    case ValueTag::FLOAT:
      stack->push(Value::Float(left.as.FLOAT - right.as.FLOAT));
      break;
    default:
      throw msl_runtime_error(where, "expected a number as right argument");
    }
  default:
    throw msl_runtime_error(where, "expected a number as left argument");
  }
}
void vm_mul(LocationRef where, Stack *stack) {
  Value right = stack->pop();
  Value left = stack->pop();
  switch (left.tag) {
  case ValueTag::INT:
    switch (right.tag) {
    case ValueTag::INT:
      stack->push(Value::Int(left.as.INT * right.as.INT));
      break;
    case ValueTag::FLOAT:
      stack->push(Value::Float(left.as.INT * right.as.FLOAT));
      break;
    default:
      throw msl_runtime_error(where, "expected a number as right argument");
    }
  case ValueTag::FLOAT:
    switch (right.tag) {
    case ValueTag::INT:
      stack->push(Value::Float(left.as.FLOAT * right.as.INT));
      break;
    case ValueTag::FLOAT:
      stack->push(Value::Float(left.as.FLOAT * right.as.FLOAT));
      break;
    default:
      throw msl_runtime_error(where, "expected a number as right argument");
    }
  default:
    throw msl_runtime_error(where, "expected a number as left argument");
  }
}
void vm_div(LocationRef where, Stack *stack) {
  Value right = stack->pop();
  Value left = stack->pop();
  switch (left.tag) {
  case ValueTag::INT:
    switch (right.tag) {
    case ValueTag::INT:
      stack->push(Value::Float(static_cast<double>(left.as.INT) /
                               static_cast<double>(right.as.INT)));
      break;
    case ValueTag::FLOAT:
      stack->push(
          Value::Float(static_cast<double>(left.as.INT) * right.as.FLOAT));
      break;
    default:
      throw msl_runtime_error(where, "expected a number as right argument");
    }
  case ValueTag::FLOAT:
    switch (right.tag) {
    case ValueTag::INT:
      stack->push(
          Value::Float(left.as.FLOAT * static_cast<double>(right.as.INT)));
      break;
    case ValueTag::FLOAT:
      stack->push(Value::Float(left.as.FLOAT * right.as.FLOAT));
      break;
    default:
      throw msl_runtime_error(where, "expected a number as right argument");
    }
  default:
    throw msl_runtime_error(where, "expected a number as left argument");
  }
}
void vm_mod(LocationRef where, Stack *stack) {
  Value right = stack->pop();
  Value left = stack->pop();
  switch (left.tag) {
  case ValueTag::INT:
    switch (right.tag) {
    case ValueTag::INT:
      stack->push(Value::Int(left.as.INT % right.as.INT));
      break;
    default:
      throw msl_runtime_error(where, "expected an integer as right argument");
    }
  default:
    throw msl_runtime_error(where, "expected an integer as left argument");
  }
}
void vm_lt(LocationRef where, Stack *stack) {
  Value right = stack->pop();
  Value left = stack->pop();
  bool is_true = false;
  switch (left.tag) {
  case ValueTag::INT:
    switch (right.tag) {
    case ValueTag::INT:
      is_true = left.as.INT < right.as.INT;
      break;
    case ValueTag::FLOAT:
      is_true = left.as.INT < right.as.FLOAT;
      break;
    default:
      throw msl_runtime_error(where, "expected a number as right argument");
    }
  case ValueTag::FLOAT:
    switch (right.tag) {
    case ValueTag::INT:
      is_true = left.as.FLOAT < right.as.INT;
      break;
    case ValueTag::FLOAT:
      is_true = left.as.FLOAT < right.as.FLOAT;
      break;
    default:
      throw msl_runtime_error(where, "expected a number as right argument");
    }
  default:
    throw msl_runtime_error(where, "expected a number as left argument");
  }
  if (is_true) {
    stack->push(Value::Symbol(Constants::SYM_TRUE));
  } else {
    stack->push(Value::Symbol(Constants::SYM_FALSE));
  }
}
void vm_lte(LocationRef where, Stack *stack) {
  Value right = stack->pop();
  Value left = stack->pop();
  bool is_true = false;
  switch (left.tag) {
  case ValueTag::INT:
    switch (right.tag) {
    case ValueTag::INT:
      is_true = left.as.INT <= right.as.INT;
      break;
    case ValueTag::FLOAT:
      is_true = left.as.INT <= right.as.FLOAT;
      break;
    default:
      throw msl_runtime_error(where, "expected a number as right argument");
    }
  case ValueTag::FLOAT:
    switch (right.tag) {
    case ValueTag::INT:
      is_true = left.as.FLOAT <= right.as.INT;
      break;
    case ValueTag::FLOAT:
      is_true = left.as.FLOAT <= right.as.FLOAT;
      break;
    default:
      throw msl_runtime_error(where, "expected a number as right argument");
    }
  default:
    throw msl_runtime_error(where, "expected a number as left argument");
  }
  if (is_true) {
    stack->push(Value::Symbol(Constants::SYM_TRUE));
  } else {
    stack->push(Value::Symbol(Constants::SYM_FALSE));
  }
}
void vm_gt(LocationRef where, Stack *stack) {
  Value right = stack->pop();
  Value left = stack->pop();
  bool is_true = false;
  switch (left.tag) {
  case ValueTag::INT:
    switch (right.tag) {
    case ValueTag::INT:
      is_true = left.as.INT > right.as.INT;
      break;
    case ValueTag::FLOAT:
      is_true = left.as.INT > right.as.FLOAT;
      break;
    default:
      throw msl_runtime_error(where, "expected a number as right argument");
    }
  case ValueTag::FLOAT:
    switch (right.tag) {
    case ValueTag::INT:
      is_true = left.as.FLOAT > right.as.INT;
      break;
    case ValueTag::FLOAT:
      is_true = left.as.FLOAT > right.as.FLOAT;
      break;
    default:
      throw msl_runtime_error(where, "expected a number as right argument");
    }
  default:
    throw msl_runtime_error(where, "expected a number as left argument");
  }
  if (is_true) {
    stack->push(Value::Symbol(Constants::SYM_TRUE));
  } else {
    stack->push(Value::Symbol(Constants::SYM_FALSE));
  }
}
void vm_gte(LocationRef where, Stack *stack) {
  Value right = stack->pop();
  Value left = stack->pop();
  bool is_true = false;
  switch (left.tag) {
  case ValueTag::INT:
    switch (right.tag) {
    case ValueTag::INT:
      is_true = left.as.INT >= right.as.INT;
      break;
    case ValueTag::FLOAT:
      is_true = left.as.INT >= right.as.FLOAT;
      break;
    default:
      throw msl_runtime_error(where, "expected a number as right argument");
    }
  case ValueTag::FLOAT:
    switch (right.tag) {
    case ValueTag::INT:
      is_true = left.as.FLOAT >= right.as.INT;
      break;
    case ValueTag::FLOAT:
      is_true = left.as.FLOAT >= right.as.FLOAT;
      break;
    default:
      throw msl_runtime_error(where, "expected a number as right argument");
    }
  default:
    throw msl_runtime_error(where, "expected a number as left argument");
  }
  if (is_true) {
    stack->push(Value::Symbol(Constants::SYM_TRUE));
  } else {
    stack->push(Value::Symbol(Constants::SYM_FALSE));
  }
}
void vm_eq(Stack *stack, VMHeap *heap) {
  Value right = stack->pop();
  Value left = stack->pop();
  bool is_true = values_equal(heap, left, right);
  if (is_true) {
    stack->push(Value::Symbol(Constants::SYM_TRUE));
  } else {
    stack->push(Value::Symbol(Constants::SYM_FALSE));
  }
}
void vm_neq(Stack *stack, VMHeap *heap) {
  Value right = stack->pop();
  Value left = stack->pop();
  bool is_true = values_equal(heap, left, right);
  if (!is_true) {
    stack->push(Value::Symbol(Constants::SYM_TRUE));
  } else {
    stack->push(Value::Symbol(Constants::SYM_FALSE));
  }
}
}; // namespace
int run(std::vector<VMInstr> instrs) {
  VMHeap heap = VMHeap(1000, 1000);
  // return addresses
  std::stack<uint64_t> ret;
  // frame pointers
  std::stack<uint64_t> fps;
  Stack stack = Stack(10000);
  uint64_t iptr = 0;
  uint64_t fptr = 0;
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
    case VMTag::STORE: {
      Value val = stack.pop();
      stack.write_local(fptr, instr.as.STKADDR, val);
      ++iptr;
    } break;
    case VMTag::STORE_GLOBAL: {
      Value val = stack.pop();
      stack.write_global(instr.as.STKADDR, val);
      ++iptr;
    } break;
    case VMTag::LOAD: {
      Value val = stack.load_local(fptr, instr.as.STKADDR);
      stack.push(val);
      ++iptr;
    } break;
    case VMTag::LOAD_GLOBAL: {
      Value val = stack.load_global(instr.as.STKADDR);
      stack.push(val);
      ++iptr;
    } break;
    case VMTag::NOT: {
      Value val = stack.pop();
      if (as_bool(instr.where, val)) {
        stack.push(Value::Symbol(Constants::SYM_FALSE));
      } else {
        stack.push(Value::Symbol(Constants::SYM_TRUE));
      }
      ++iptr;
    } break;
    case VMTag::INVERT: {
      Value val = stack.pop();
      switch (val.tag) {
      case ValueTag::INT:
        stack.push(Value::Int(val.as.INT * -1));
        break;
      case ValueTag::FLOAT:
        stack.push(Value::Float(val.as.FLOAT * -1));
        break;
      default:
        throw msl_runtime_error(instr.where, "expected a number");
      }
      ++iptr;
    } break;
    case VMTag::ADD:
      vm_add(instr.where, &stack);
      ++iptr;
      break;
    case VMTag::SUB:
      vm_sub(instr.where, &stack);
      ++iptr;
      break;
    case VMTag::MUL:
      vm_mul(instr.where, &stack);
      ++iptr;
      break;
    case VMTag::DIV:
      vm_div(instr.where, &stack);
      ++iptr;
      break;
    case VMTag::MOD:
      vm_mod(instr.where, &stack);
      ++iptr;
      break;
    case VMTag::LT:
      vm_lt(instr.where, &stack);
      ++iptr;
      break;
    case VMTag::LTE:
      vm_lte(instr.where, &stack);
      ++iptr;
      break;
    case VMTag::GT:
      vm_gt(instr.where, &stack);
      ++iptr;
      break;
    case VMTag::GTE:
      vm_gte(instr.where, &stack);
      ++iptr;
      break;
    case VMTag::EQ:
      vm_eq(&stack, &heap);
      ++iptr;
      break;
    case VMTag::NEQ:
      vm_neq(&stack, &heap);
      ++iptr;
      break;
    case VMTag::DUP:
      stack.dup();
      ++iptr;
    case VMTag::TYPEOF: {
      Value value = stack.pop();
      switch (value.tag) {
      case ValueTag::INT:
        stack.push(Value::Symbol(Constants::SYM_T_INT));
        break;
      case ValueTag::FLOAT:
        stack.push(Value::Symbol(Constants::SYM_T_FLOAT));
        break;
      case ValueTag::SYMBOL:
        stack.push(Value::Symbol(Constants::SYM_T_SYMBOL));
        break;
      case ValueTag::STRING:
        stack.push(Value::Symbol(Constants::SYM_T_STRING));
        break;
      case ValueTag::LIST:
        stack.push(Value::Symbol(Constants::SYM_T_LIST));
        break;
      case ValueTag::ERROR:
        stack.push(Value::Symbol(Constants::SYM_T_ERROR));
        break;
      case ValueTag::NONE:
        stack.push(Value::Symbol(Constants::SYM_T_NONE));
        break;
      }
      ++iptr;
    } break;
    case VMTag::CALL:
      ret.push(iptr + 1);
      iptr = instr.as.INSTRADDR.addr;
      break;
    case VMTag::VMCALL: {
      Value val = vm_fns.at(instr.as.VMFN.index)(instr.where, &stack, &heap,
                                                 instr.extra.args);
      stack.push(val);
      ++iptr;
    } break;
    case VMTag::INIT_FRAME: {
      fps.push(stack.stkptr);
      stack.allocate(instr.extra.locals);
      ++iptr;
    } break;
    case VMTag::RETURN: {
      uint64_t prev_frame = fps.top();
      fps.pop();
      Value return_value = stack.pop();
      stack.retreat(prev_frame);
      stack.push(return_value);
      iptr = ret.top();
      ret.pop();
    } break;
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
