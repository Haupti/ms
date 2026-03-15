#include "core.hpp"
#include "../../lib/asap/util.hpp"
#include "../msl_runtime_error.hpp"
#include "stack.hpp"
#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <random>
#include <sstream>
#include <string>
namespace {
std::vector<std::string> msl_args;
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
  }
}

Value type_to_symbol(const Value &value) {
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
ValueTag symbol_to_type(LocationRef where, const Symbol &symbol) {
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
  } else {
    throw msl_runtime_error(where, "'" + resolve_symbol(symbol) +
                                       "' does not denote a type");
  }
}

void assert_list(LocationRef where, Value value) {
  if (value.tag != ValueTag::LIST) {
    throw msl_runtime_error(where, "expected a list but got a(n) " +
                                       type_to_string(value.tag));
  }
}
void assert_int(LocationRef where, Value value) {
  if (value.tag != ValueTag::INT) {
    throw msl_runtime_error(where, "expected an int but got a(n) " +
                                       type_to_string(value.tag));
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
  return type_to_symbol(value);
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
      return Value::Error(
          heap->add(heap->add_string("could not convert string to int")));
    }
  }
  default:
    throw msl_runtime_error(where, "cannot convert " + type_to_string(val.tag) +
                                       " to int");
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
      return Value::Error(heap->add(
          heap->add_string("could not convert '" +
                           heap->get_string(val.as.STRING) + "' to float")));
    }
  }
  default:
    throw msl_runtime_error(where, "cannot convert " + type_to_string(val.tag) +
                                       " to float");
  }
}

Value core::value_to_string_fn(LocationRef, Stack *stack, VMHeap *heap) {
  Value val = stack->pop();
  return heap->add_string(value_to_string(stack, heap, val));
}

Value core::file_read(LocationRef where, Stack *stack, VMHeap *heap) {
  Value path_val = stack->pop();
  if (path_val.tag != ValueTag::STRING) {
    throw msl_runtime_error(where, "expected a string for file path");
  }
  std::string path = heap->get_string(path_val.as.STRING);
  std::ifstream f(path);
  if (!f.is_open()) {
    return Value::Error(
        heap->add(heap->add_string("could not open file: " + path)));
  }
  std::stringstream buffer;
  buffer << f.rdbuf();
  return heap->add_string(buffer.str());
}

Value core::file_write(LocationRef where, Stack *stack, VMHeap *heap) {
  Value content_val = stack->pop();
  Value path_val = stack->pop();
  if (path_val.tag != ValueTag::STRING || content_val.tag != ValueTag::STRING) {
    throw msl_runtime_error(where, "expected strings for path and content");
  }
  std::string path = heap->get_string(path_val.as.STRING);
  std::string content = heap->get_string(content_val.as.STRING);
  std::ofstream f(path);
  if (!f.is_open()) {
    return Value::Error(
        heap->add(heap->add_string("could not open file: " + path)));
  }
  f << content;
  return Value::None();
}

Value core::file_append(LocationRef where, Stack *stack, VMHeap *heap) {
  Value content_val = stack->pop();
  Value path_val = stack->pop();
  if (path_val.tag != ValueTag::STRING || content_val.tag != ValueTag::STRING) {
    throw msl_runtime_error(where, "expected strings for path and content");
  }
  std::string path = heap->get_string(path_val.as.STRING);
  std::string content = heap->get_string(content_val.as.STRING);
  std::ofstream f(path, std::ios::app);
  if (!f.is_open()) {
    return Value::Error(
        heap->add(heap->add_string("could not open file: " + path)));
  }
  f << content;
  return Value::None();
}

Value core::sys_env(LocationRef where, Stack *stack, VMHeap *heap) {
  Value name_val = stack->pop();
  if (name_val.tag != ValueTag::STRING) {
    throw msl_runtime_error(where,
                            "expected a string for environment variable name");
  }
  const char *val = std::getenv(heap->get_string(name_val.as.STRING).c_str());
  if (val == nullptr) {
    return Value::Error(
        heap->add(heap->add_string("environment variable not found: " +
                                   heap->get_string(name_val.as.STRING))));
  }
  return heap->add_string(std::string(val));
}

Value core::process_args(LocationRef, Stack *, VMHeap *heap) {
  VMHIDX list_head = heap->new_list();
  for (const auto &arg : ::msl_args) {
    heap->add_child(list_head, heap->add_string(arg));
  }
  return heap->at(list_head);
}

void core::set_args(const std::vector<std::string> &args) { ::msl_args = args; }

Value core::sys_exit(LocationRef where, Stack *stack, VMHeap *) {
  Value code_val = stack->pop();
  if (code_val.tag != ValueTag::INT) {
    throw msl_runtime_error(where, "expected an integer for exit code");
  }
  std::exit(static_cast<int>(code_val.as.INT));
}

Value core::sys_exec(LocationRef, Stack *stack, VMHeap *heap) {
  int64_t args_count = stack->pop().as.INT;
  std::string command = "";
  for (int64_t i = 0; i < args_count; ++i) {
    command =
        value_to_string(stack, heap, stack->pop()).append(" ").append(command);
  }
  int result = std::system(command.c_str());
  return Value::Int(result);
}

Value core::random_int(LocationRef where, Stack *stack, VMHeap *) {
  Value max_val = stack->pop();
  Value min_val = stack->pop();
  if (min_val.tag != ValueTag::INT || max_val.tag != ValueTag::INT) {
    throw msl_runtime_error(where, "expected integers for random range");
  }
  if (min_val.as.INT > max_val.as.INT) {
    throw msl_runtime_error(where, "min must be less than or equal to max");
  }

  static std::random_device rd;
  static std::mt19937 gen(rd());
  std::uniform_int_distribution<int64_t> distrib(min_val.as.INT,
                                                 max_val.as.INT);

  return Value::Int(distrib(gen));
}

Value core::time_epoch_ms(LocationRef, Stack *, VMHeap *) {
  auto now = std::chrono::system_clock::now();
  auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                now.time_since_epoch())
                .count();
  return Value::Int(ms);
}

Value core::time_epoch_sec(LocationRef, Stack *, VMHeap *) {
  auto now = std::chrono::system_clock::now();
  auto sec =
      std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch())
          .count();
  return Value::Int(sec);
}

Value core::time_iso8601(LocationRef, Stack *, VMHeap *heap) {
  auto now = std::chrono::system_clock::now();
  std::time_t now_c = std::chrono::system_clock::to_time_t(now);
  std::tm now_tm;
#ifdef _WIN32
  gmtime_s(&now_tm, &now_c);
#else
  gmtime_r(&now_c, &now_tm);
#endif
  std::stringstream ss;
  ss << std::put_time(&now_tm, "%Y-%m-%dT%H:%M:%SZ");
  return heap->add_string(ss.str());
}

Value core::str_split(LocationRef where, Stack *stack, VMHeap *heap) {
  Value delim_val = stack->pop();
  Value str_val = stack->pop();
  if (str_val.tag != ValueTag::STRING || delim_val.tag != ValueTag::STRING) {
    throw msl_runtime_error(where, "expected strings for str_split");
  }
  std::string s = heap->get_string(str_val.as.STRING);
  std::string delim = heap->get_string(delim_val.as.STRING);
  VMHIDX list_head = heap->new_list();
  if (delim.empty()) {
    for (char c : s) {
      heap->add_child(list_head, heap->add_string(std::string(1, c)));
    }
  } else {
    size_t start = 0;
    size_t end = s.find(delim);
    while (end != std::string::npos) {
      heap->add_child(list_head,
                      heap->add_string(s.substr(start, end - start)));
      start = end + delim.length();
      end = s.find(delim, start);
    }
    heap->add_child(list_head, heap->add_string(s.substr(start)));
  }
  return heap->at(list_head);
}

Value core::str_replace(LocationRef where, Stack *stack, VMHeap *heap) {
  Value replacement_val = stack->pop();
  Value search_val = stack->pop();
  Value str_val = stack->pop();
  if (str_val.tag != ValueTag::STRING || search_val.tag != ValueTag::STRING ||
      replacement_val.tag != ValueTag::STRING) {
    throw msl_runtime_error(where, "expected strings for str_replace");
  }
  std::string s = heap->get_string(str_val.as.STRING);
  std::string search = heap->get_string(search_val.as.STRING);
  std::string replacement = heap->get_string(replacement_val.as.STRING);
  if (search.empty())
    return str_val;
  size_t pos = 0;
  while ((pos = s.find(search, pos)) != std::string::npos) {
    s.replace(pos, search.length(), replacement);
    pos += replacement.length();
  }
  return heap->add_string(s);
}

Value core::str_contains(LocationRef where, Stack *stack, VMHeap *heap) {
  Value sub_val = stack->pop();
  Value str_val = stack->pop();
  if (str_val.tag != ValueTag::STRING || sub_val.tag != ValueTag::STRING) {
    throw msl_runtime_error(where, "expected strings for str_contains");
  }
  std::string s = heap->get_string(str_val.as.STRING);
  std::string sub = heap->get_string(sub_val.as.STRING);
  bool found = s.find(sub) != std::string::npos;
  return Value::Symbol(found ? Constants::SYM_TRUE : Constants::SYM_FALSE);
}

Value core::str_has_prefix(LocationRef where, Stack *stack, VMHeap *heap) {
  Value prefix_val = stack->pop();
  Value str_val = stack->pop();
  if (str_val.tag != ValueTag::STRING || prefix_val.tag != ValueTag::STRING) {
    throw msl_runtime_error(where, "expected strings for str_has_prefix");
  }
  std::string s = heap->get_string(str_val.as.STRING);
  std::string prefix = heap->get_string(prefix_val.as.STRING);
  bool res = s.compare(0, prefix.length(), prefix) == 0;
  return Value::Symbol(res ? Constants::SYM_TRUE : Constants::SYM_FALSE);
}

Value core::str_has_suffix(LocationRef where, Stack *stack, VMHeap *heap) {
  Value suffix_val = stack->pop();
  Value str_val = stack->pop();
  if (str_val.tag != ValueTag::STRING || suffix_val.tag != ValueTag::STRING) {
    throw msl_runtime_error(where, "expected strings for str_has_suffix");
  }
  std::string s = heap->get_string(str_val.as.STRING);
  std::string suffix = heap->get_string(suffix_val.as.STRING);
  bool res = false;
  if (s.length() >= suffix.length()) {
    res = s.compare(s.length() - suffix.length(), suffix.length(), suffix) == 0;
  }
  return Value::Symbol(res ? Constants::SYM_TRUE : Constants::SYM_FALSE);
}

Value core::str_lower(LocationRef where, Stack *stack, VMHeap *heap) {
  Value str_val = stack->pop();
  if (str_val.tag != ValueTag::STRING) {
    throw msl_runtime_error(where, "expected string for str_lower");
  }
  std::string s = heap->get_string(str_val.as.STRING);
  std::transform(s.begin(), s.end(), s.begin(),
                 [](unsigned char c) { return std::tolower(c); });
  return heap->add_string(s);
}

Value core::str_upper(LocationRef where, Stack *stack, VMHeap *heap) {
  Value str_val = stack->pop();
  if (str_val.tag != ValueTag::STRING) {
    throw msl_runtime_error(where, "expected string for str_upper");
  }
  std::string s = heap->get_string(str_val.as.STRING);
  std::transform(s.begin(), s.end(), s.begin(),
                 [](unsigned char c) { return std::toupper(c); });
  return heap->add_string(s);
}

Value core::str_trim(LocationRef where, Stack *stack, VMHeap *heap) {
  Value str_val = stack->pop();
  if (str_val.tag != ValueTag::STRING) {
    throw msl_runtime_error(where, "expected string for str_trim");
  }
  std::string s = heap->get_string(str_val.as.STRING);
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
            return !std::isspace(ch);
          }));
  s.erase(std::find_if(s.rbegin(), s.rend(),
                       [](unsigned char ch) { return !std::isspace(ch); })
              .base(),
          s.end());
  return heap->add_string(s);
}
Value core::vmassert(LocationRef where, Stack *stack, VMHeap *) {
  Value value = stack->pop();
  if (!core::as_bool(where, value)) {
    throw msl_runtime_error(where, "assertion failed");
  }
  return Value::None();
}
Value core::vmassert_type(LocationRef where, Stack *stack, VMHeap *) {
  Value typeval = stack->pop();
  Value val = stack->pop();
  if (val.tag != ValueTag::SYMBOL) {
    throw msl_runtime_error(where, "expected a symbol as second argument");
  }
  ValueTag actual = symbol_to_type(where, val.as.SYMBOL);
  if (typeval.tag != actual) {
    throw msl_runtime_error(where,
                            "expected a " + resolve_symbol(typeval.as.SYMBOL) +
                                " but got " + type_to_string(actual) + "");
  }
  return Value::None();
}
