#include "../msl_runtime_error.hpp"
#include "constants.hpp"
#include "core.hpp"
#include "stack.hpp"
#include "value_and_heap.hpp"
#include "vm_instr.hpp"
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

// VM INSTRUCTIONS
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
    break;
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
    break;
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
    break;
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
    break;
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
    break;
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
    break;
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
          Value::Float(static_cast<double>(left.as.INT) / right.as.FLOAT));
      break;
    default:
      throw msl_runtime_error(where, "expected a number as right argument");
    }
    break;
  case ValueTag::FLOAT:
    switch (right.tag) {
    case ValueTag::INT:
      stack->push(
          Value::Float(left.as.FLOAT / static_cast<double>(right.as.INT)));
      break;
    case ValueTag::FLOAT:
      stack->push(Value::Float(left.as.FLOAT / right.as.FLOAT));
      break;
    default:
      throw msl_runtime_error(where, "expected a number as right argument");
    }
    break;
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
    break;
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
    break;
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
    break;
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
    break;
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
    break;
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
    break;
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
    break;
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
    break;
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
    break;
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
  bool is_true = core::values_equal(heap, left, right);
  if (is_true) {
    stack->push(Value::Symbol(Constants::SYM_TRUE));
  } else {
    stack->push(Value::Symbol(Constants::SYM_FALSE));
  }
}
void vm_neq(Stack *stack, VMHeap *heap) {
  Value right = stack->pop();
  Value left = stack->pop();
  bool is_true = core::values_equal(heap, left, right);
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
      break;
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
      Value val = core::fns.at(instr.as.VMFN.index)(instr.where, &stack, &heap,
                                                    instr.extra.args);
      stack.push(val);
      ++iptr;
    } break;
    case VMTag::INIT_FRAME: {
      fps.push(fptr);
      fptr = stack.stkptr - instr.as.INT;
      stack.allocate(instr.extra.locals);
      ++iptr;
    } break;
    case VMTag::RETURN: {
      uint64_t prev_frame = fps.top();
      fps.pop();
      Value return_value = stack.pop();
      stack.retreat(fptr); // Retreat to start of current frame
      fptr = prev_frame;   // Restore previous frame pointer
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
