#include "core.hpp"
#include "../../lib/asap/util.hpp"
#include "../msl_runtime_error.hpp"
#include "stack.hpp"
#include <string>
namespace {
inline std::string value_tag_to_string(ValueTag tag) {
  switch (tag) {
  case ValueTag::INT:
    return "int";
  case ValueTag::FLOAT:
    return "float";
  case ValueTag::SYMBOL:
    return "symbol";
  case ValueTag::STRING:
    return "string";
  case ValueTag::LIST:
    return "list";
  case ValueTag::ERROR:
    return "error";
  case ValueTag::NONE:
    return "none";
  }
}
void assert_list(LocationRef where, Value value) {
  if (value.tag != ValueTag::LIST) {
    throw msl_runtime_error(where, "expected a list but got a(n) " +
                                       value_tag_to_string(value.tag));
  }
}
void assert_int(LocationRef where, Value value) {
  if (value.tag != ValueTag::INT) {
    throw msl_runtime_error(where, "expected an int but got a(n) " +
                                       value_tag_to_string(value.tag));
  }
}

Value internal_copy_value(Stack *stack, VMHeap *heap, Value value) {
  switch (value.tag) {
  case ValueTag::INT:
    return value;
  case ValueTag::FLOAT:
    return value;
  case ValueTag::SYMBOL:
    return value;
  case ValueTag::STRING: {
    std::string underlying_string = heap->get_string(value.as.STRING);
    return heap->add_string(underlying_string);
  } break;
  case ValueTag::LIST: {
    VMHIDX new_list = heap->new_list();
    VMHIDX curr = heap->node_at(value.as.LIST)->first_child;
    while (curr != INVALID) {
      heap->add_child(new_list, heap->at(curr));
      curr = heap->node_at(curr)->next_child;
    }
    return heap->at(new_list);
  } break;
  case ValueTag::ERROR: {
    Value underlying_value = heap->at(value.as.ERROR);
    VMHIDX underlying_value_idx =
        heap->add(internal_copy_value(stack, heap, underlying_value));
    return Value::Error(underlying_value_idx);
  } break;
  case ValueTag::NONE:
    return value;
  }
}

}; // namespace
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
  case ValueTag::LIST: {
    std::vector<std::string> args;
    uint64_t i = 0;
    Value val = heap->nth_child(value.as.LIST, i);
    while (!val.undefined) {
      args.push_back(value_to_string(stack, heap, val));
      ++i;
      val = heap->nth_child(value.as.LIST, i);
    }
    return "list(" + join(args, ",") + ")";
  }
  case ValueTag::ERROR:
    return "error(" + value_to_string(stack, heap, heap->at(value.as.ERROR)) +
           ")";
  case ValueTag::NONE:
    return "none";
  }
}

Value core::print(LocationRef, Stack *stack, VMHeap *heap) {
  println(value_to_string(stack, heap, stack->pop()));
  return Value::None();
}
Value core::make_error(LocationRef, Stack *stack, VMHeap *heap) {
  Value inner = stack->pop();
  return Value::Error(heap->add(inner));
}
[[noreturn]] Value core::vmpanic(LocationRef where, Stack *stack,
                                 VMHeap *heap) {
  throw msl_runtime_error(where, value_to_string(stack, heap, stack->pop()));
}
Value core::list(LocationRef, Stack *stack, VMHeap *heap) {
  int64_t args_count = stack->pop().as.INT;
  VMHIDX list_head = heap->new_list();
  for (int64_t i = 0; i < args_count; ++i) {
    heap->add_child_front(list_head, stack->pop());
  }
  return heap->at(list_head);
}
Value core::list_at(LocationRef where, Stack *stack, VMHeap *heap) {
  Value index_value = stack->pop();
  assert_int(where, index_value);
  Value list_value = stack->pop();
  assert_list(where, list_value);
  return heap->nth_child(list_value.as.LIST, index_value.as.INT);
}
Value core::list_put(LocationRef where, Stack *stack, VMHeap *heap) {
  Value new_value = stack->pop();
  Value index_value = stack->pop();
  assert_int(where, index_value);
  Value list_value = stack->pop();
  assert_list(where, list_value);
  heap->set_nth_child(list_value.as.LIST, index_value.as.INT, new_value);
  return Value::None();
}
Value core::list_append(LocationRef where, Stack *stack, VMHeap *heap) {
  Value value = stack->pop();
  Value list = stack->pop();
  assert_list(where, list);
  heap->add_child(list.as.LIST, value);
  return list;
}
Value core::list_prepend(LocationRef where, Stack *stack, VMHeap *heap) {
  Value value = stack->pop();
  Value list = stack->pop();
  assert_list(where, list);
  heap->add_child_front(list.as.LIST, value);
  return list;
}
Value core::list_link(LocationRef where, Stack *stack, VMHeap *heap) {
  Value right_list = stack->pop();
  assert_list(where, right_list);
  Value left_list = stack->pop();
  assert_list(where, left_list);
  if (left_list.as.LIST == right_list.as.LIST) {
    throw msl_runtime_error(
        where,
        "linking a list to itself is undefined behaviour. in this specific "
        "case i caught it, but i cannot save you from every mistake.");
  }
  heap->link_lists(left_list.as.LIST, right_list.as.LIST);
  return left_list;
}

Value core::value_copy(LocationRef, Stack *stack, VMHeap *heap) {
  Value value = stack->pop();
  return internal_copy_value(stack, heap, value);
}
Value core::vmtypeof(LocationRef, Stack *stack, VMHeap *) {
  Value value = stack->pop();
  switch (value.tag) {
  case ValueTag::INT:
    return (Value::Symbol(Constants::SYM_T_INT));
  case ValueTag::FLOAT:
    return (Value::Symbol(Constants::SYM_T_FLOAT));
  case ValueTag::SYMBOL:
    return (Value::Symbol(Constants::SYM_T_SYMBOL));
  case ValueTag::STRING:
    return (Value::Symbol(Constants::SYM_T_STRING));
  case ValueTag::LIST:
    return (Value::Symbol(Constants::SYM_T_LIST));
  case ValueTag::ERROR:
    return (Value::Symbol(Constants::SYM_T_ERROR));
  case ValueTag::NONE:
    return (Value::Symbol(Constants::SYM_T_NONE));
  }
}
Value core::string_concat(LocationRef where, Stack *stack, VMHeap *heap) {
  Value right = stack->pop();
  Value left = stack->pop();
  if (left.tag != ValueTag::STRING) {
    throw msl_runtime_error(where, "expected string as left argument");
  }
  if (right.tag != ValueTag::STRING) {
    throw msl_runtime_error(where, "expected strings as right argument");
  }
  std::string s1 = heap->get_string(left.as.STRING);
  std::string s2 = heap->get_string(right.as.STRING);
  return heap->add_string(s1 + s2);
}
Value core::len(LocationRef where, Stack *stack, VMHeap *heap) {
  Value val = stack->pop();
  if (val.tag == ValueTag::STRING) {
    return Value::Int(heap->get_string(val.as.STRING).length());
  } else if (val.tag == ValueTag::LIST) {
    uint64_t count = 0;
    VMHIDX curr = heap->node_at(val.as.LIST)->first_child;
    while (curr != INVALID) {
      count++;
      curr = heap->node_at(curr)->next_child;
    }
    return Value::Int(count);
  } else {
    throw msl_runtime_error(where, "expected string or list for len()");
  }
}

Value core::value_to_int(LocationRef where, Stack *stack, VMHeap *heap) {
  Value val = stack->pop();
  switch (val.tag) {
  case ValueTag::INT:
    return val;
  case ValueTag::FLOAT:
    return Value::Int(static_cast<int64_t>(val.as.FLOAT));
  case ValueTag::STRING: {
    try {
      return Value::Int(std::stoll(heap->get_string(val.as.STRING)));
    } catch (...) {
      throw msl_runtime_error(where, "could not convert string to int");
    }
  }
  default:
    throw msl_runtime_error(
        where, "cannot convert " + value_tag_to_string(val.tag) + " to int");
  }
}

Value core::value_to_float(LocationRef where, Stack *stack, VMHeap *heap) {
  Value val = stack->pop();
  switch (val.tag) {
  case ValueTag::INT:
    return Value::Float(static_cast<double>(val.as.INT));
  case ValueTag::FLOAT:
    return val;
  case ValueTag::STRING: {
    try {
      return Value::Float(std::stod(heap->get_string(val.as.STRING)));
    } catch (...) {
      throw msl_runtime_error(where, "could not convert '" +
                                         heap->get_string(val.as.STRING) +
                                         "' to float");
    }
  }
  default:
    throw msl_runtime_error(
        where, "cannot convert " + value_tag_to_string(val.tag) + " to float");
  }
}

Value core::value_to_string_fn(LocationRef, Stack *stack, VMHeap *heap) {
  Value val = stack->pop();
  return heap->add_string(value_to_string(stack, heap, val));
}
