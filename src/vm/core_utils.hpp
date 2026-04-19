#pragma once
#include "../location.hpp"
#include "../msl_runtime_error.hpp"
#include "constants.hpp"
#include "stack.hpp"
#include "value_and_heap.hpp"

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/ioctl.h>
#include <unistd.h>
#endif

namespace core_utils {

struct TerminalSize {
  int width;
  int height;
  bool success;
};

inline TerminalSize get_terminal_size() {
  TerminalSize size = {0, 0, false};

#ifdef _WIN32
  CONSOLE_SCREEN_BUFFER_INFO csbi;
  if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
    size.width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    size.height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    size.success = true;
  } else {
    size.success = false;
  }
#else
  struct winsize w;
  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) != -1) {
    size.width = w.ws_col;
    size.height = w.ws_row;
    size.success = true;
  } else {
    size.success = false;
  }
#endif
  return size;
}
inline Value type_to_symbol(const Value &value) {
  switch (value.tag) {
  case ValueTag::INT:
    return (Value::FromSymbol(Constants::SYM_T_INT));
  case ValueTag::FLOAT:
    return (Value::FromSymbol(Constants::SYM_T_FLOAT));
  case ValueTag::SYMBOL:
    return (Value::FromSymbol(Constants::SYM_T_SYMBOL));
  case ValueTag::STRING:
    return (Value::FromSymbol(Constants::SYM_T_STRING));
  case ValueTag::LIST:
    return (Value::FromSymbol(Constants::SYM_T_LIST));
  case ValueTag::ERROR:
    return (Value::FromSymbol(Constants::SYM_T_ERROR));
  case ValueTag::BOX:
    return (Value::FromSymbol(Constants::SYM_T_BOX));
  case ValueTag::NONE:
    return (Value::FromSymbol(Constants::SYM_T_NONE));
  case ValueTag::ITERATOR:
    return (Value::FromSymbol(Constants::SYM_T_ITERATOR));
  case ValueTag::TABLE:
    return (Value::FromSymbol(Constants::SYM_T_TABLE));
  case ValueTag::FN_REF:
    return (Value::FromSymbol(Constants::SYM_T_FN_REF));
  }
  return Value::None();
}
inline bool as_bool(LocationRef ref, Value value) {
  switch (value.tag) {
  case ValueTag::SYMBOL:
    return value.as.SYMBOL.index == Constants::SYM_TRUE.index;
  default:
    throw msl_runtime_error(ref, "expected a symbol");
  }
}
inline std::string as_string(LocationRef ref, Value value, VMHeap *heap) {
  switch (value.tag) {
  case ValueTag::STRING:
    return heap->get_string(value.as.STRING);
  default:
    throw msl_runtime_error(ref, "expected a string");
    break;
  }
}
inline VMHIDX as_table(LocationRef ref, Value value, VMHeap *) {
  switch (value.tag) {
  case ValueTag::TABLE:
    return value.as.TABLE;
  default:
    throw msl_runtime_error(ref, "expected a table");
    break;
  }
}
inline Value to_bool(bool b) {
  if (b) {
    return Value::FromSymbol(Constants::SYM_TRUE);
  }
  return Value::FromSymbol(Constants::SYM_FALSE);
}

inline int64_t as_int(LocationRef ref, Value value) {
  switch (value.tag) {
  case ValueTag::INT:
    return value.as.INT;
  default:
    throw msl_runtime_error(ref, "expected an integer");
    break;
  }
}
inline Value create_error(VMHeap *heap, const std::string &str) {
  return Value::Error(heap->ref_add_string(str));
}

inline ValueTag symbol_to_type(LocationRef where, const Symbol &symbol) {
  if (symbol.index == Constants::SYM_T_INT.index) {
    return ValueTag::INT;
  } else if (symbol.index == Constants::SYM_T_FLOAT.index) {
    return ValueTag::FLOAT;
  } else if (symbol.index == Constants::SYM_T_STRING.index) {
    return ValueTag::STRING;
  } else if (symbol.index == Constants::SYM_T_SYMBOL.index) {
    return ValueTag::SYMBOL;
  } else if (symbol.index == Constants::SYM_T_NONE.index) {
    return ValueTag::NONE;
  } else if (symbol.index == Constants::SYM_T_LIST.index) {
    return ValueTag::LIST;
  } else if (symbol.index == Constants::SYM_T_ERROR.index) {
    return ValueTag::ERROR;
  } else if (symbol.index == Constants::SYM_T_ITERATOR.index) {
    return ValueTag::ITERATOR;
  } else if (symbol.index == Constants::SYM_T_TABLE.index) {
    return ValueTag::TABLE;
  } else if (symbol.index == Constants::SYM_T_FN_REF.index) {
    return ValueTag::FN_REF;
  } else {
    throw msl_runtime_error(where, "'" + resolve_symbol(symbol) +
                                       "' does not denote a type");
  }
}
inline std::string type_to_string(ValueTag tag) {
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
  case ValueTag::ITERATOR:
    return "iterator";
  case ValueTag::TABLE:
    return "table";
  case ValueTag::FN_REF:
    return "function-reference";
  case ValueTag::BOX:
    return "box";
  }
}
inline void assert_list(LocationRef where, Value value) {
  if (value.tag != ValueTag::LIST) {
    throw msl_runtime_error(where, "expected a list but got a(n) " +
                                       type_to_string(value.tag));
  }
}
inline void assert_table(LocationRef where, Value value) {
  if (value.tag != ValueTag::TABLE) {
    throw msl_runtime_error(where, "expected a table but got a(n) " +
                                       type_to_string(value.tag));
  }
}
inline void assert_int(LocationRef where, Value value) {
  if (value.tag != ValueTag::INT) {
    throw msl_runtime_error(where, "expected an int but got a(n) " +
                                       type_to_string(value.tag));
  }
}
inline void assert_sym(LocationRef where, Value value) {
  if (value.tag != ValueTag::SYMBOL) {
    throw msl_runtime_error(where, "expected a symbol but got a(n) " +
                                       type_to_string(value.tag));
  }
}
inline void assert_fn_ref(LocationRef where, Value value) {
  if (value.tag != ValueTag::FN_REF) {
    throw msl_runtime_error(where,
                            "expected a function reference but got a(n) " +
                                type_to_string(value.tag));
  }
}
inline void assert_str(LocationRef where, Value value) {
  if (value.tag != ValueTag::STRING) {
    throw msl_runtime_error(where, "expected a string but got a(n) " +
                                       type_to_string(value.tag));
  }
}

inline Value copy_value(Stack *stack, VMHeap *heap, Value value) {
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
      heap->add_child(new_list, copy_value(stack, heap, heap->at(curr)));
      curr = heap->node_at(curr)->next_child;
    }
    return heap->at(new_list);
  } break;
  case ValueTag::ERROR: {
    Value underlying_value = heap->at(value.as.ERROR);
    VMHIDX underlying_value_idx =
        heap->add(copy_value(stack, heap, underlying_value));
    return Value::Error(underlying_value_idx);
  } break;
  case ValueTag::BOX: {
    Value underlying_value = heap->at(value.as.BOX);
    VMHIDX underlying_value_idx =
        heap->add(copy_value(stack, heap, underlying_value));
    return Value::Box(underlying_value_idx);
  } break;
  case ValueTag::NONE:
    return value;
  case ValueTag::ITERATOR:
    // an iterator is a view into a list thus a copy stays the same
    return value;
  case ValueTag::TABLE: {
    VMHIDX new_table = heap->new_table();
    VMHIDX curr = heap->node_at(value.as.TABLE)->first_child;
    while (curr != INVALID) {
      Value copied_pair = copy_value(stack, heap, heap->at(curr));
      heap->link_existing_child(new_table, copied_pair.as.LIST);
      curr = heap->node_at(curr)->next_child;
    }
    return heap->at(new_table);
  } break;
  case ValueTag::FN_REF: {
    Value underlying_value = heap->at(value.as.FN_REF);
    VMHIDX underlying_value_idx =
        heap->add(copy_value(stack, heap, underlying_value));
    return Value::FunctionReference(underlying_value_idx);
  } break;
  }
}

inline std::string value_to_string(Stack *stack, VMHeap *heap, Value value) {
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
  case ValueTag::BOX:
    return "box(" + value_to_string(stack, heap, heap->at(value.as.BOX)) + ")";
  case ValueTag::NONE:
    return "none";
  case ValueTag::ITERATOR:
    return "iterator(" + std::to_string(value.as.ITERATOR) + ")";
  case ValueTag::TABLE: {
    std::vector<std::string> args;
    VMHIDX curr = heap->node_at(value.as.TABLE)->first_child;
    while (curr != INVALID) {
      args.push_back(value_to_string(stack, heap, heap->nth_child(curr, 0)));
      args.push_back(value_to_string(stack, heap, heap->nth_child(curr, 1)));
      curr = heap->node_at(curr)->next_child;
    }
    return "table(" + join(args, ",") + ")";
  } break;
  case ValueTag::FN_REF: {
    return "ref(" + value_to_string(stack, heap, heap->at(value.as.FN_REF)) +
           ")";
  } break;
  }
}


inline void table_add_entry(VMHeap *heap, VMHIDX table, Value key,
                            Value value) {
  VMHIDX new_pair = heap->new_list();
  heap->add_child(new_pair, key);
  heap->add_child(new_pair, value);
  heap->link_existing_child(table, new_pair);
}

inline void table_set_entry(VMHeap *heap, VMHIDX table, Value key,
                            Value value) {
  VMHIDX idx = heap->nth_child_idx(table, 0);
  while (idx != INVALID) {
    Value current_key = heap->nth_child(idx, 0);
    if (core::values_equal(heap, key, current_key)) {
      heap->replace(heap->nth_child_idx(idx, 1), value);
      return;
    }
    idx = heap->next_child(idx);
  }
  table_add_entry(heap, table, key, value);
}
inline void table_add_entry_front(VMHeap *heap, VMHIDX table, Value key,
                                  Value value) {
  VMHIDX new_pair = heap->new_list();
  heap->add_child(new_pair, key);
  heap->add_child(new_pair, value);
  heap->link_existing_child_front(table, new_pair);
}
}; // namespace core_utils
