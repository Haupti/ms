#include "core.hpp"
#include "../msl_runtime_error.hpp"
#include "stack.hpp"
#include <string>
bool core::values_equal(VMHeap *heap, Value left, Value right) {
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

std::string core::value_to_string(Stack *stack, VMHeap *heap, Value value) {
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

Value core::print(LocationRef where, Stack *stack, VMHeap *heap,
                  uint16_t args) {
  if (args != 1) {
    throw msl_runtime_error(where, "expects 1 argument(s)");
  }
  println(value_to_string(stack, heap, stack->pop()));
  return Value::None();
}
Value core::make_error(LocationRef where, Stack *stack, VMHeap *heap,
                       uint16_t args) {
  if (args != 1) {
    throw msl_runtime_error(where, "expects 1 argument(s)");
  }
  Value inner = stack->pop();
  return Value::Error(heap->add(inner));
}
[[noreturn]] Value core::vmpanic(LocationRef where, Stack *stack, VMHeap *heap,
                                 uint16_t args) {
  if (args != 1) {
    throw msl_runtime_error(where, "expects 1 argument(s)");
  }
  throw msl_runtime_error(where, value_to_string(stack, heap, stack->pop()));
}
