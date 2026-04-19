#include "vm.hpp"
#include "../msl_runtime_error.hpp"
#include "constants.hpp"
#include "core.hpp"
#include "core_utils.hpp"
#include "garbage_collection.hpp"
#include "stack.hpp"
#include "value_and_heap.hpp"
#include "vm_instr.hpp"
#include <vector>

namespace {
// GARBAGE COLLECTION TRIGGERS
void run_gc_on_metric(VMHeap *heap, Stack *stack) {
  if (heap->current_length > (heap->reserved_length / 2)) {
    run_gc(heap, stack);
  } else {
    return;
  }
  if (heap->current_length > (heap->reserved_length * 0.7)) {
    heap->reserve_factor(1.5);
  }
}

// VM INSTRUCTIONS
inline void vm_add(LocationRef where, Stack *stack) {
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
inline void vm_sub(LocationRef where, Stack *stack) {
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
inline void vm_mul(LocationRef where, Stack *stack) {
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
inline void vm_div(LocationRef where, Stack *stack) {
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
inline void vm_mod(LocationRef where, Stack *stack) {
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
inline void vm_lt(LocationRef where, Stack *stack) {
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
    stack->push(Value::FromSymbol(Constants::SYM_TRUE));
  } else {
    stack->push(Value::FromSymbol(Constants::SYM_FALSE));
  }
}
inline void vm_lte(LocationRef where, Stack *stack) {
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
    stack->push(Value::FromSymbol(Constants::SYM_TRUE));
  } else {
    stack->push(Value::FromSymbol(Constants::SYM_FALSE));
  }
}
inline void vm_gt(LocationRef where, Stack *stack) {
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
    stack->push(Value::FromSymbol(Constants::SYM_TRUE));
  } else {
    stack->push(Value::FromSymbol(Constants::SYM_FALSE));
  }
}
inline void vm_gte(LocationRef where, Stack *stack) {
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
    stack->push(Value::FromSymbol(Constants::SYM_TRUE));
  } else {
    stack->push(Value::FromSymbol(Constants::SYM_FALSE));
  }
}
inline void vm_eq(Stack *stack, VMHeap *heap) {
  Value right = stack->pop();
  Value left = stack->pop();
  bool is_true = core::values_equal(heap, left, right);
  if (is_true) {
    stack->push(Value::FromSymbol(Constants::SYM_TRUE));
  } else {
    stack->push(Value::FromSymbol(Constants::SYM_FALSE));
  }
}
inline void vm_neq(Stack *stack, VMHeap *heap) {
  Value right = stack->pop();
  Value left = stack->pop();
  bool is_true = core::values_equal(heap, left, right);
  if (!is_true) {
    stack->push(Value::FromSymbol(Constants::SYM_TRUE));
  } else {
    stack->push(Value::FromSymbol(Constants::SYM_FALSE));
  }
}
} // namespace

VM::VM(std::vector<VMInstr> instrs, const std::vector<std::string> &msl_args) : stack(10000), heap(1000, 1000), instrs(instrs) {
  core::set_args(msl_args);
}

bool VM::step() {
  if (iptr >= instrs.size())
    return false;
  const VMInstr &instr = instrs[iptr];
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
    run_gc_on_metric(&heap, &stack);
    stack.push(heap.add_string(instr.as.STRING));
    ++iptr;
    break;
  case VMTag::PUSH_SYMBOL:
    stack.push(Value::FromSymbol(instr.as.SYMBOL));
    ++iptr;
    break;
  case VMTag::PUSH_NONE:
    stack.push(Value::None());
    ++iptr;
    break;
  case VMTag::PUSH_FN_REF: {
    stack.push(heap.new_fn_ref(instr.as.INSTRADDR, instr.extra.args));
    ++iptr;
  } break;
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
    if (core_utils::as_bool(instr.where, val)) {
      stack.push(Value::FromSymbol(Constants::SYM_FALSE));
    } else {
      stack.push(Value::FromSymbol(Constants::SYM_TRUE));
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
    stack.push(core::vmtypeof(instr.where, this));
    ++iptr;
  } break;
  case VMTag::CALL:
    run_gc_on_metric(&heap, &stack);
    ret_stack.push_back(iptr + 1);
    iptr = instr.as.INSTRADDR.addr;
    break;
  case VMTag::VMCALL: {
    run_gc_on_metric(&heap, &stack);
    Value val = core::fns.at(instr.as.FUNC.index)(instr.where, this);
    stack.push(val);
    ++iptr;
  } break;
  case VMTag::REFINVOKE: {
    run_gc_on_metric(&heap, &stack);
    ret_stack.push_back(iptr + 1);
    Value args_count = stack.pop();
    Value val = stack.pop();
    if (val.tag != ValueTag::FN_REF) {
      throw msl_runtime_error(instr.where,
                              "expected a function reference but got '" +
                                  core_utils::type_to_string(val.tag) + "'");
    }
    InstrAddr addr;
    uint16_t args;
    heap.get_fn_ref(val.as.FN_REF, &addr, &args);
    if (args_count.as.INT != args) {
      throw msl_runtime_error(instr.where,
                              "expected '" + std::to_string(args) +
                                  "' arguments but got '" +
                                  std::to_string(args_count.as.INT) + "'");
    }
    iptr = addr.addr;
  } break;
  case VMTag::INIT_FRAME: {
    fps_stack.push_back(fptr);
    fptr = stack.stkptr - instr.as.INT;
    stack.allocate(instr.extra.locals);
    ++iptr;
  } break;
  case VMTag::RETURN: {
    uint64_t prev_frame = fps_stack.back();
    fps_stack.pop_back();
    Value return_value = stack.pop();
    stack.retreat(fptr); // Retreat to start of current frame
    fptr = prev_frame;   // Restore previous frame pointer
    stack.push(return_value);
    iptr = ret_stack.back();
    ret_stack.pop_back();
  } break;
  case VMTag::POP: {
    stack.pop_void();
    ++iptr;
  } break;
  case VMTag::ITER_FOREACH: {
    Value list = stack.pop();
    switch (list.tag) {
    case ValueTag::LIST: {
      if (heap.nth_child_idx(list.as.LIST, 0) != INVALID) {
        VMHIDX first_elem_idx = heap.nth_child_idx(list.as.LIST, 0);
        stack.push(Value::Iterator(first_elem_idx));
        stack.push(heap.at(first_elem_idx));
        ++iptr;
      } else {
        iptr = instr.as.INSTRADDR.addr;
      }
    } break;
    case ValueTag::ITERATOR: {
      if (heap.node_at(list.as.ITERATOR)->next_child != INVALID) {
        VMHIDX next_elem_idx = heap.node_at(list.as.ITERATOR)->next_child;
        stack.push(Value::Iterator(next_elem_idx));
        stack.push(heap.at(next_elem_idx));
        ++iptr;
      } else {
        iptr = instr.as.INSTRADDR.addr;
      }
    } break;
    default:
      throw msl_runtime_error(instr.where, "value not iterable");
    }
  } break;
  case VMTag::PEEK_JMPIF: {
    if (core_utils::as_bool(instr.where, stack.peek())) {
      iptr = instr.as.INSTRADDR.addr;
    } else {
      ++iptr;
    }
  } break;
  case VMTag::PEEK_JMPIFN: {
    if (!core_utils::as_bool(instr.where, stack.peek())) {
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
    if (!core_utils::as_bool(instr.where, val)) {
      iptr = instr.as.INSTRADDR.addr;
    } else {
      ++iptr;
    }
  } break;
  case VMTag::JMPIF: {
    Value val = stack.pop();
    if (core_utils::as_bool(instr.where, val)) {
      iptr = instr.as.INSTRADDR.addr;
    } else {
      ++iptr;
    }
  } break;
  case VMTag::JMP: {
    iptr = instr.as.INSTRADDR.addr;
  } break;
  case VMTag::HALT:
    return false;
  }
  return true;
}

int VM::run() {
  while (iptr < instrs.size()) {
    if (!step())
      break;
  }
  return 0;
}

Value VM::call_reference(Value fn_ref, std::vector<Value> args) {
  if (fn_ref.tag != ValueTag::FN_REF) {
    return core_utils::create_error(&heap,
                                    "attempt to call non-function reference");
  }

  InstrAddr addr;
  uint16_t expected_args;
  heap.get_fn_ref(fn_ref.as.FN_REF, &addr, &expected_args);

  if (args.size() != expected_args) {
    return core_utils::create_error(&heap, "argument count mismatch");
  }

  uint64_t saved_iptr = iptr;
  for (const auto &arg : args) {
    stack.push(arg);
  }

  iptr = addr.addr;
  // We push a sentinel to the return stack.
  // The VM's RETURN will eventually pop this and set iptr to it.
  ret_stack.push_back(REF_CALL_RETURN_IPTR);

  while (iptr != REF_CALL_RETURN_IPTR) {
    if (!step())
      break;
  }

  // Restore the instruction pointer to where we were before the call.
  iptr = saved_iptr;
  return stack.pop();
}

int run(std::vector<VMInstr> instrs, const std::vector<std::string> &msl_args) {
  VM vm(instrs, msl_args);
  return vm.run();
}
