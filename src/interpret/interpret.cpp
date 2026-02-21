#include "../../lib/asap/util.hpp"
#include "../../tests/testutil_node_to_string.hpp"
#include "../interned_string.hpp"
#include "../msl_runtime_error.hpp"
#include "../parser/node.hpp"
#include <unordered_map>

using namespace std;
namespace {
// ------- TYPES -------
// ------- TYPES -------
// ------- TYPES -------

enum class ValueTag : uint8_t {
  INT,
  STRING,
  FLOAT,
  SYMBOL,
  LIST,
  NONE,
};

struct HeapNodeIdx {
  uint64_t idx;
  bool is_null() { return idx == 0; }
  HeapNodeIdx(uint64_t idx) : idx(idx) {}
  HeapNodeIdx() {}
};

enum class Mark : uint8_t {
  STATIC,
  FREE,
  MARKED,
};

// lists:
// Value of type lists holds heap node of type list
// i.e. value list holds a 'pointer' to a heap list
// which itself can contain heap list 'pointers'
struct HeapNode {
  uint64_t next_sibling;
  uint64_t first_child;
  uint64_t next_child;
  union {
    int64_t INT;
    double FLOAT;
    Symbol SYMBOL;
    HeapNodeIdx LIST;
    string *STRING;
  } as;
  ValueTag tag;
};

struct Value {
  union {
    int64_t INT;
    HeapNodeIdx STRING;
    double FLOAT;
    Symbol SYMBOL;
    HeapNodeIdx LIST;
  } as;
  ValueTag tag;
  Mark mark;
  Value() : as({0}), tag(ValueTag::NONE) {}
  static Value Int(int64_t value) {
    Value val;
    val.as.INT = value;
    val.tag = ValueTag::INT;
    return val;
  }
  static Value Float(double value) {
    Value val;
    val.as.FLOAT = value;
    val.tag = ValueTag::FLOAT;
    return val;
  }
  static Value Symbol(Symbol value) {
    Value val;
    val.as.SYMBOL = value;
    val.tag = ValueTag::SYMBOL;
    return val;
  }
  static Value None() {
    Value val;
    val.tag = ValueTag::NONE;
    return val;
  }
};

struct ListIterator {
  Value current;
  HeapNodeIdx next;
};

struct Heap {
  vector<HeapNode> elements;
  HeapNodeIdx free_list = HeapNodeIdx(0);
  unordered_map<uint64_t, HeapNodeIdx> static_strings;

  Value at(HeapNodeIdx idx) { return to_stack(elements.at(idx.idx), idx); }
  HeapNodeIdx alloc(Value value) {
    if (value.tag == ValueTag::STRING) {
      return value.as.STRING;
    }
    if (free_list.is_null()) {
      elements.push_back(to_heap(value));
      return HeapNodeIdx(elements.size() - 1);
    } else {
      HeapNodeIdx current_free = free_list;
      free_list = elements.at(free_list.idx).next_sibling;
      elements[current_free.idx] = to_heap(value);
      return current_free;
    }
  }
  Value alloc_new_empty_list() {
    HeapNode node = {};
    node.as.LIST = HeapNodeIdx{0};
    node.tag = ValueTag::LIST;
    HeapNodeIdx list_idx = alloc_next_free(node);
    Value value = Value();
    value.as.LIST = list_idx;
    value.tag = ValueTag::LIST;
    return value;
  }
  void list_append(HeapNodeIdx list_idx, Value value) {
    HeapNode list_node = elements.at(list_idx.idx);
    HeapNode list_element_node = to_heap(value);
    HeapNodeIdx list_element_idx = alloc_next_free(list_element_node);
    if (list_node.first_child == 0) {
      elements.at(list_idx.idx).first_child = list_element_idx.idx;
      return;
    }
    uint64_t last_child = list_node.first_child;
    while (elements.at(last_child).next_child != 0) {
      last_child = elements.at(last_child).next_child;
    }
    elements.at(last_child).next_child = list_element_idx.idx;
  }
  ListIterator get_list_iterator(HeapNodeIdx first_elem) {
    ListIterator it;
    it.current = at(first_elem);
    HeapNode list_head = elements.at(first_elem.idx);
    it.next = list_head.first_child;
    return it;
  }
  void list_iterator_advance(ListIterator *it) {
    it->current = at(it->next);
    HeapNodeIdx next_elem = elements.at(it->next.idx).next_child;
    it->next = next_elem;
  }
  Value alloc_new_string(const string &str) {
    HeapNode node = {};
    node.as.STRING = new string(str);
    node.tag = ValueTag::STRING;
    HeapNodeIdx new_string_idx = alloc_next_free(node);
    Value value;
    value.as.STRING = new_string_idx;
    value.tag = ValueTag::STRING;
    return value;
  }
  Value alloc_new_string(InternedString str) {
    if (static_strings.count(str.index) > 0) {
      Value value;
      value.as.STRING = static_strings.at(str.index);
      value.tag = ValueTag::STRING;
      return value;
    }
    HeapNode node = {};
    node.as.STRING = new string(resolve_interned_string(str));
    node.tag = ValueTag::STRING;
    HeapNodeIdx new_string_idx = alloc_next_free(node);

    Value value;
    value.as.STRING = new_string_idx;
    value.tag = ValueTag::STRING;
    static_strings[str.index] = new_string_idx;
    return value;
  }
  string heap_string(HeapNodeIdx idx) {
    string aaa = *elements.at(idx.idx).as.STRING;
    return aaa;
  }
  Heap() { elements.reserve(10000); }

private:
  HeapNodeIdx alloc_next_free(HeapNode node) {
    if (free_list.is_null()) {
      elements.push_back(node);
      return HeapNodeIdx(elements.size() - 1);
    }
    HeapNodeIdx current_free = free_list;
    free_list = elements.at(free_list.idx).next_sibling;
    elements[current_free.idx] = node;
    return current_free;
  }
  HeapNode to_heap(const Value &value) {
    HeapNode node = {};
    node.tag = value.tag;
    switch (value.tag) {
    case ValueTag::INT:
      node.as.INT = value.as.INT;
      break;
    case ValueTag::STRING:
      return elements.at(value.as.STRING.idx);
      break;
    case ValueTag::FLOAT:
      node.as.FLOAT = value.as.FLOAT;
      break;
    case ValueTag::SYMBOL:
      node.as.SYMBOL = value.as.SYMBOL;
      break;
    case ValueTag::LIST:
      node.as.LIST = value.as.LIST;
      break;
    case ValueTag::NONE:
      break;
    }
    return node;
  }
  Value to_stack(const HeapNode &node, HeapNodeIdx node_idx) {
    Value val;
    val.tag = node.tag;
    switch (val.tag) {
    case ValueTag::INT:
      val.as.INT = node.as.INT;
      break;
    case ValueTag::STRING:
      val.as.STRING = node_idx;
      break;
    case ValueTag::FLOAT:
      val.as.FLOAT = node.as.FLOAT;
      break;
    case ValueTag::SYMBOL:
      val.as.SYMBOL = node.as.SYMBOL;
      break;
    case ValueTag::LIST:
      val.as.LIST = node.as.LIST;
      break;
    case ValueTag::NONE:
      break;
    }
    return val;
  }
};

struct GlobalContext {
  // key is InternedString unpacked
  unordered_map<uint64_t, node_idx> functions;
  unordered_map<uint64_t, Value> variables;
};
struct Scope {
  unordered_map<uint64_t, Value> variables;
  Value return_value = Value::None();
  Scope *parent = nullptr;
  bool is_returning = false;
  bool has_var(InternedString varname) {
    if (variables.count(varname.index) > 0) {
      return true;
    }
    if (parent) {
      return parent->has_var(varname);
    }
    return false;
  }
  Value get_var(InternedString varname) {
    if (variables.count(varname.index) > 0) {
      return variables[varname.index];
    } else {
      return parent->get_var(varname);
    }
  }
  void set_var(InternedString varname, Value value) {
    if (variables.count(varname.index) > 0) {
      variables[varname.index] = value;
    } else {
      parent->set_var(varname, value);
    }
  }
};

// ------- GLOBALS -------
// ------- GLOBALS -------
// ------- GLOBALS -------

static nodes _PROG;
static GlobalContext _GLOBAL;
static Heap _HEAP;
static Symbol _SYM_TRUE = create_symbol("#true");
static Symbol _SYM_T = create_symbol("#t");
static Symbol _SYM_FALSE = create_symbol("#false");
static InternedString _BUILDIN_FN_PRINT = create_interned_string("print");
static InternedString _BUILDIN_FN_LIST = create_interned_string("list");
static InternedString _BUILDIN_FN_PUT = create_interned_string("put");
static InternedString _BUILDIN_FN_AT = create_interned_string("at");

// ------- LOGIC -------
// ------- LOGIC -------
// ------- LOGIC -------

// ------- UTILS -------
// ------- UTILS -------
// ------- UTILS -------
inline bool value_as_bool(const Value &value) {
  // TODO either that or throw if its not #true/#t/#f/#false not sure yet
  return value.tag == ValueTag::SYMBOL &&
         (value.as.SYMBOL.index == _SYM_TRUE.index ||
          value.as.SYMBOL.index == _SYM_T.index);
}

Value interpret_expression(Scope *scope, node_idx node);
// ------- BUILDIN FUNCTIONS -------
// ------- BUILDIN FUNCTIONS -------
// ------- BUILDIN FUNCTIONS -------
Value msl_buildin_list(Scope *scope, Node node) {
  Value list = _HEAP.alloc_new_empty_list();
  node_idx list_element = _PROG.at(node.first_child).next_child;
  while (!list_element.is_null()) {
    Value elem = interpret_expression(scope, list_element);
    _HEAP.list_append(list.as.LIST, elem);
    list_element = _PROG.at(list_element).next_child;
  }
  return list;
}
Value msl_buildin_put(Scope *scope, Node node) {
  // TODO implement
  if (scope && node.first_child.is_null()) {
    throw runtime_error("NOT YET IMPLEMENTED");
  }
  throw runtime_error("NOT YET IMPLEMENTED");
}
Value msl_buildin_at(Scope *scope, Node node) {
  // TODO implement
  if (scope && node.first_child.is_null()) {
    throw runtime_error("NOT YET IMPLEMENTED");
  }
  throw runtime_error("NOT YET IMPLEMENTED");
}
string value_to_string(const Value &value) {
  switch (value.tag) {
  case ValueTag::INT:
    return to_string(value.as.INT);
  case ValueTag::STRING:
    return _HEAP.heap_string(value.as.STRING);
  case ValueTag::FLOAT:
    return to_string(value.as.FLOAT);
  case ValueTag::SYMBOL:
    return resolve_symbol(value.as.SYMBOL);
  case ValueTag::LIST: {
    vector<string> elems;
    ListIterator iterator = _HEAP.get_list_iterator(value.as.LIST);
    _HEAP.list_iterator_advance(&iterator);
    while (!iterator.next.is_null()) {
      elems.push_back(value_to_string(iterator.current));
      _HEAP.list_iterator_advance(&iterator);
    }
    return "[" + join(elems, ",") + "]";
  };
  case ValueTag::NONE:
    return "none";
  }
}
Value msl_buildin_println(Scope *scope, Node node) {
  Value arg =
      interpret_expression(scope, _PROG.at(node.first_child).next_child);
  println(value_to_string(arg));
  return Value::None();
}
// --------- OPERATORS

inline Value interpret_operator_str_concat(Scope *scope, Node curr) {
  Value leftval = interpret_expression(scope, curr.first_child);
  Value rightval =
      interpret_expression(scope, _PROG.at(curr.first_child).next_child);
  if (leftval.tag != ValueTag::STRING || rightval.tag != ValueTag::STRING) {
    throw msl_runtime_error(curr.start, "'<>' expected two strings");
  }
  return _HEAP.alloc_new_string(_HEAP.heap_string(leftval.as.STRING) +
                                _HEAP.heap_string(rightval.as.STRING));
}
inline Value interpret_operator_neq(Scope *scope, Node curr) {
  Value leftval = interpret_expression(scope, curr.first_child);
  Value rightval =
      interpret_expression(scope, _PROG.at(curr.first_child).next_child);
  switch (leftval.tag) {
  case ValueTag::INT:
    switch (rightval.tag) {
    case ValueTag::INT:
      if (leftval.as.INT == rightval.as.INT) {
        return Value::Symbol(_SYM_FALSE);
      } else {
        return Value::Symbol(_SYM_TRUE);
      }
    case ValueTag::FLOAT:
      if (leftval.as.INT == rightval.as.FLOAT) {
        return Value::Symbol(_SYM_FALSE);
      } else {
        return Value::Symbol(_SYM_TRUE);
      }
    default:
      return Value::Symbol(_SYM_TRUE);
    }
  case ValueTag::STRING:
    switch (rightval.tag) {
    case ValueTag::STRING:
      if (_HEAP.heap_string(leftval.as.STRING) ==
          _HEAP.heap_string(rightval.as.STRING)) {
        return Value::Symbol(_SYM_FALSE);
      } else {
        return Value::Symbol(_SYM_TRUE);
      }
    default:
      return Value::Symbol(_SYM_TRUE);
    }
  case ValueTag::FLOAT:
    switch (rightval.tag) {
    case ValueTag::INT:
      if (leftval.as.FLOAT == rightval.as.INT) {
        return Value::Symbol(_SYM_FALSE);
      } else {
        return Value::Symbol(_SYM_TRUE);
      }
    case ValueTag::FLOAT:
      if (leftval.as.FLOAT == rightval.as.FLOAT) {
        return Value::Symbol(_SYM_FALSE);
      } else {
        return Value::Symbol(_SYM_TRUE);
      }
    default:
      return Value::Symbol(_SYM_TRUE);
    }
  case ValueTag::SYMBOL:
    switch (rightval.tag) {
    case ValueTag::SYMBOL:
      if (leftval.as.SYMBOL.index == rightval.as.SYMBOL.index) {
        return Value::Symbol(_SYM_FALSE);
      } else {
        return Value::Symbol(_SYM_TRUE);
      }
    default:
      return Value::Symbol(_SYM_TRUE);
    }
  case ValueTag::LIST:
    if (rightval.tag == ValueTag::LIST &&
        leftval.as.LIST.idx == rightval.as.LIST.idx) {
      return Value::Symbol(_SYM_FALSE);
    } else {
      return Value::Symbol(_SYM_TRUE);
    }
  case ValueTag::NONE:
    if (rightval.tag == ValueTag::NONE) {
      return Value::Symbol(_SYM_FALSE);
    } else {
      return Value::Symbol(_SYM_TRUE);
    }
  }
}
inline Value interpret_operator_eq(Scope *scope, Node curr) {
  Value leftval = interpret_expression(scope, curr.first_child);
  Value rightval =
      interpret_expression(scope, _PROG.at(curr.first_child).next_child);
  switch (leftval.tag) {
  case ValueTag::INT:
    switch (rightval.tag) {
    case ValueTag::INT:
      if (leftval.as.INT == rightval.as.INT) {
        return Value::Symbol(_SYM_TRUE);
      } else {
        return Value::Symbol(_SYM_FALSE);
      }
    case ValueTag::FLOAT:
      if (leftval.as.INT == rightval.as.FLOAT) {
        return Value::Symbol(_SYM_TRUE);
      } else {
        return Value::Symbol(_SYM_FALSE);
      }
    default:
      return Value::Symbol(_SYM_FALSE);
    }
  case ValueTag::STRING:
    switch (rightval.tag) {
    case ValueTag::STRING:
      if (_HEAP.heap_string(leftval.as.STRING) ==
          _HEAP.heap_string(rightval.as.STRING)) {
        return Value::Symbol(_SYM_TRUE);
      } else {
        return Value::Symbol(_SYM_FALSE);
      }
    default:
      return Value::Symbol(_SYM_FALSE);
    }
  case ValueTag::FLOAT:
    switch (rightval.tag) {
    case ValueTag::INT:
      if (leftval.as.FLOAT == rightval.as.INT) {
        return Value::Symbol(_SYM_TRUE);
      } else {
        return Value::Symbol(_SYM_FALSE);
      }
    case ValueTag::FLOAT:
      if (leftval.as.FLOAT == rightval.as.FLOAT) {
        return Value::Symbol(_SYM_TRUE);
      } else {
        return Value::Symbol(_SYM_FALSE);
      }
    default:
      return Value::Symbol(_SYM_FALSE);
    }
  case ValueTag::SYMBOL:
    switch (rightval.tag) {
    case ValueTag::SYMBOL:
      if (leftval.as.SYMBOL.index == rightval.as.SYMBOL.index) {
        return Value::Symbol(_SYM_TRUE);
      } else {
        return Value::Symbol(_SYM_FALSE);
      }
    default:
      return Value::Symbol(_SYM_FALSE);
    }
  case ValueTag::LIST:
    if (rightval.tag == ValueTag::LIST &&
        leftval.as.LIST.idx == rightval.as.LIST.idx) {
      return Value::Symbol(_SYM_TRUE);
    } else {
      return Value::Symbol(_SYM_FALSE);
    }
  case ValueTag::NONE:
    if (rightval.tag == ValueTag::NONE) {
      return Value::Symbol(_SYM_TRUE);
    } else {
      return Value::Symbol(_SYM_FALSE);
    }
  }
}
inline Value interpret_operator_gte(Scope *scope, Node curr) {
  Value leftval = interpret_expression(scope, curr.first_child);
  Value rightval =
      interpret_expression(scope, _PROG.at(curr.first_child).next_child);
  switch (leftval.tag) {
  case ValueTag::INT: {
    switch (rightval.tag) {
    case ValueTag::INT:
      return Value::Int(leftval.as.INT >= rightval.as.INT);
    case ValueTag::FLOAT:
      return Value::Float(leftval.as.INT >= rightval.as.FLOAT);
    default:
      throw msl_runtime_error(curr.start, "'>=' expected two number arguments");
    }
  } break;

  case ValueTag::FLOAT: {
    switch (rightval.tag) {
    case ValueTag::INT:
      return Value::Float(leftval.as.FLOAT >= rightval.as.INT);
    case ValueTag::FLOAT:
      return Value::Float(leftval.as.FLOAT >= rightval.as.FLOAT);
    default:
      throw msl_runtime_error(curr.start, "'>=' expected two number arguments");
    }
  } break;
  default:
    throw msl_runtime_error(curr.start, "'>' expected two number arguments");
  }
}
inline Value interpret_operator_gt(Scope *scope, Node curr) {
  Value leftval = interpret_expression(scope, curr.first_child);
  Value rightval =
      interpret_expression(scope, _PROG.at(curr.first_child).next_child);
  switch (leftval.tag) {
  case ValueTag::INT: {
    switch (rightval.tag) {
    case ValueTag::INT:
      return Value::Int(leftval.as.INT > rightval.as.INT);
    case ValueTag::FLOAT:
      return Value::Float(leftval.as.INT > rightval.as.FLOAT);
    default:
      throw msl_runtime_error(curr.start, "'>' expected two number arguments");
    }
  } break;

  case ValueTag::FLOAT: {
    switch (rightval.tag) {
    case ValueTag::INT:
      return Value::Float(leftval.as.FLOAT > rightval.as.INT);
    case ValueTag::FLOAT:
      return Value::Float(leftval.as.FLOAT > rightval.as.FLOAT);
    default:
      throw msl_runtime_error(curr.start, "'>' expected two number arguments");
    }
  } break;
  default:
    throw msl_runtime_error(curr.start, "'>' expected two number arguments");
  }
}
inline Value interpret_operator_lte(Scope *scope, Node curr) {
  Value leftval = interpret_expression(scope, curr.first_child);
  Value rightval =
      interpret_expression(scope, _PROG.at(curr.first_child).next_child);
  switch (leftval.tag) {
  case ValueTag::INT: {
    switch (rightval.tag) {
    case ValueTag::INT:
      return Value::Int(leftval.as.INT <= rightval.as.INT);
    case ValueTag::FLOAT:
      return Value::Float(leftval.as.INT <= rightval.as.FLOAT);
    default:
      throw msl_runtime_error(curr.start, "'<=' expected two number arguments");
    }
  } break;

  case ValueTag::FLOAT: {
    switch (rightval.tag) {
    case ValueTag::INT:
      return Value::Float(leftval.as.FLOAT <= rightval.as.INT);
    case ValueTag::FLOAT:
      return Value::Float(leftval.as.FLOAT <= rightval.as.FLOAT);
    default:
      throw msl_runtime_error(curr.start, "'<=' expected two number arguments");
    }
  } break;
  default:
    throw msl_runtime_error(curr.start, "'<=' expected two number arguments");
  }
}
inline Value interpret_operator_lt(Scope *scope, Node curr) {
  Value leftval = interpret_expression(scope, curr.first_child);
  Value rightval =
      interpret_expression(scope, _PROG.at(curr.first_child).next_child);
  switch (leftval.tag) {
  case ValueTag::INT: {
    switch (rightval.tag) {
    case ValueTag::INT:
      return Value::Int(leftval.as.INT < rightval.as.INT);
    case ValueTag::FLOAT:
      return Value::Float(leftval.as.INT < rightval.as.FLOAT);
    default:
      throw msl_runtime_error(curr.start, "'<' expected two number arguments");
    }
  } break;

  case ValueTag::FLOAT: {
    switch (rightval.tag) {
    case ValueTag::INT:
      return Value::Float(leftval.as.FLOAT < rightval.as.INT);
    case ValueTag::FLOAT:
      return Value::Float(leftval.as.FLOAT < rightval.as.FLOAT);
    default:
      throw msl_runtime_error(curr.start, "'<' expected two number arguments");
    }
  } break;
  default:
    throw msl_runtime_error(curr.start, "'<' expected two number arguments");
  }
}
inline Value interpret_operator_or(Scope *scope, Node curr) {
  Value leftval = interpret_expression(scope, curr.first_child);
  if (leftval.tag == ValueTag::SYMBOL &&
      leftval.as.SYMBOL.index == _SYM_TRUE.index) {
    return Value::Symbol(_SYM_TRUE);
  }
  Value rightval =
      interpret_expression(scope, _PROG.at(curr.first_child).next_child);
  if (rightval.tag == ValueTag::SYMBOL &&
      rightval.as.SYMBOL.index == _SYM_TRUE.index) {
    return Value::Symbol(_SYM_TRUE);
  } else {
    return Value::Symbol(_SYM_FALSE);
  }
}
inline Value interpret_operator_and(Scope *scope, Node curr) {
  Value leftval = interpret_expression(scope, curr.first_child);
  if (leftval.tag == ValueTag::SYMBOL &&
      leftval.as.SYMBOL.index != _SYM_TRUE.index) {
    return Value::Symbol(_SYM_FALSE);
  } else {
    return Value::Symbol(_SYM_FALSE);
  }
  Value rightval =
      interpret_expression(scope, _PROG.at(curr.first_child).next_child);
  if (rightval.tag == ValueTag::SYMBOL &&
      rightval.as.SYMBOL.index == _SYM_TRUE.index) {
    return Value::Symbol(_SYM_TRUE);
  } else {
    return Value::Symbol(_SYM_FALSE);
  }
}
inline Value interpret_operator_mod(Scope *scope, Node curr) {
  Value leftval = interpret_expression(scope, curr.first_child);
  Value rightval =
      interpret_expression(scope, _PROG.at(curr.first_child).next_child);
  switch (leftval.tag) {
  case ValueTag::INT: {
    switch (rightval.tag) {
    case ValueTag::INT:
      return Value::Int(leftval.as.INT % rightval.as.INT);
    default:
      throw msl_runtime_error(curr.start,
                              "'mod' expected two integer arguments");
    }
  } break;
  default:
    throw msl_runtime_error(curr.start, "'mod' expected two number arguments");
  }
}
inline Value interpret_operator_div(Scope *scope, Node curr) {
  Value leftval = interpret_expression(scope, curr.first_child);
  Value rightval =
      interpret_expression(scope, _PROG.at(curr.first_child).next_child);
  switch (leftval.tag) {
  case ValueTag::INT: {
    switch (rightval.tag) {
    case ValueTag::INT:
      return Value::Float(static_cast<double>(leftval.as.INT) /
                          static_cast<double>(rightval.as.INT));
    case ValueTag::FLOAT:
      return Value::Float(static_cast<double>(leftval.as.INT) /
                          rightval.as.FLOAT);
    default:
      throw msl_runtime_error(curr.start, "'/' expected two number arguments");
    }
  } break;

  case ValueTag::FLOAT: {
    switch (rightval.tag) {
    case ValueTag::INT:
      return Value::Float(leftval.as.FLOAT /
                          static_cast<double>(rightval.as.INT));
    case ValueTag::FLOAT:
      return Value::Float(leftval.as.FLOAT / rightval.as.FLOAT);
    default:
      throw msl_runtime_error(curr.start, "'/' expected two number arguments");
    }
  } break;
  default:
    throw msl_runtime_error(curr.start, "'/' expected two number arguments");
  }
}
inline Value interpret_operator_mul(Scope *scope, Node curr) {
  Value leftval = interpret_expression(scope, curr.first_child);
  Value rightval =
      interpret_expression(scope, _PROG.at(curr.first_child).next_child);
  switch (leftval.tag) {
  case ValueTag::INT: {
    switch (rightval.tag) {
    case ValueTag::INT:
      return Value::Int(leftval.as.INT * rightval.as.INT);
    case ValueTag::FLOAT:
      return Value::Float(leftval.as.INT * rightval.as.FLOAT);
    default:
      throw msl_runtime_error(curr.start, "'*' expected two number arguments");
    }
  } break;

  case ValueTag::FLOAT: {
    switch (rightval.tag) {
    case ValueTag::INT:
      return Value::Float(leftval.as.FLOAT * rightval.as.INT);
    case ValueTag::FLOAT:
      return Value::Float(leftval.as.FLOAT * rightval.as.FLOAT);
    default:
      throw msl_runtime_error(curr.start, "'*' expected two number arguments");
    }
  } break;
  default:
    throw msl_runtime_error(curr.start, "'*' expected two number arguments");
  }
}

inline Value interpret_operator_sub(Scope *scope, Node curr) {
  Value leftval = interpret_expression(scope, curr.first_child);
  Value rightval =
      interpret_expression(scope, _PROG.at(curr.first_child).next_child);
  switch (leftval.tag) {
  case ValueTag::INT: {
    switch (rightval.tag) {
    case ValueTag::INT:
      return Value::Int(leftval.as.INT - rightval.as.INT);
    case ValueTag::FLOAT:
      return Value::Float(leftval.as.INT - rightval.as.FLOAT);
    default:
      throw msl_runtime_error(curr.start, "'-' expected two number arguments");
    }
  } break;

  case ValueTag::FLOAT: {
    switch (rightval.tag) {
    case ValueTag::INT:
      return Value::Float(leftval.as.FLOAT - rightval.as.INT);
    case ValueTag::FLOAT:
      return Value::Float(leftval.as.FLOAT - rightval.as.FLOAT);
    default:
      throw msl_runtime_error(curr.start, "'-' expected two number arguments");
    }
  } break;
  default:
    throw msl_runtime_error(curr.start, "'-' expected two number arguments");
  }
}
inline Value interpret_operator_invers(Scope *scope, Node curr) {
  Value val = interpret_expression(scope, curr.first_child);
  switch (val.tag) {
  case ValueTag::INT:
    return Value::Int(-1 * val.as.INT);
  case ValueTag::FLOAT:
    return Value::Float(-1 * val.as.FLOAT);
  default:
    throw msl_runtime_error(curr.start, "'-' expected a number");
  }
}
inline Value interpret_operator_not(Scope *scope, Node curr) {
  Value val = interpret_expression(scope, curr.first_child);
  if (val.tag == ValueTag::SYMBOL && val.as.SYMBOL.index == _SYM_TRUE.index) {
    return Value::Symbol(_SYM_FALSE);
  } else if (val.tag == ValueTag::SYMBOL) {
    return Value::Symbol(_SYM_TRUE);
  } else if (val.tag == ValueTag::NONE) {
    return Value::Symbol(_SYM_TRUE);
  } else {
    throw msl_runtime_error(curr.start, "'not' expected a symbol or none");
  }
}
inline Value interpret_operator_add(Scope *scope, Node curr) {
  Value leftval = interpret_expression(scope, curr.first_child);
  Value rightval =
      interpret_expression(scope, _PROG.at(curr.first_child).next_child);
  switch (leftval.tag) {
  case ValueTag::INT: {
    switch (rightval.tag) {
    case ValueTag::INT:
      return Value::Int(leftval.as.INT + rightval.as.INT);
    case ValueTag::FLOAT:
      return Value::Float(leftval.as.INT + rightval.as.FLOAT);
    default:
      throw msl_runtime_error(curr.start, "'+' expected two number arguments");
    }
  } break;

  case ValueTag::FLOAT: {
    switch (rightval.tag) {
    case ValueTag::INT:
      return Value::Float(leftval.as.FLOAT + rightval.as.INT);
    case ValueTag::FLOAT:
      return Value::Float(leftval.as.FLOAT + rightval.as.FLOAT);
    default:
      throw msl_runtime_error(curr.start, "'+' expected two number arguments");
    }
  } break;
  default:
    throw msl_runtime_error(curr.start, "'+' expected two number arguments");
  }
}

// ---------- MAIN LOGIC
void interpret_one(Scope *scope, node_idx node);

inline Value interpret_fn_call(Scope *scope, Node curr) {
  InternedString fn_name = _PROG.at(curr.first_child).as.IDENTIFIER;
  if (fn_name.index == _BUILDIN_FN_PRINT.index) {
    return msl_buildin_println(scope, curr);
  } else if (fn_name.index == _BUILDIN_FN_LIST.index) {
    return msl_buildin_list(scope, curr);
  } else if (fn_name.index == _BUILDIN_FN_PUT.index) {
    return msl_buildin_put(scope, curr);
  } else if (fn_name.index == _BUILDIN_FN_AT.index) {
    return msl_buildin_at(scope, curr);
  }
  Scope fn_scope;
  fn_scope.parent = scope;
  node_idx function = _GLOBAL.functions.at(fn_name.index);
  Node fn = _PROG.at(function);

  // first element is a list of args, first list element is first arg
  node_idx arg_name = _PROG.at(fn.first_child).first_child;
  node_idx arg = _PROG.at(curr.first_child).next_child;
  if (!arg_name.is_null()) {
    while (!arg_name.is_null()) {
      if (arg.is_null()) {
        throw msl_runtime_error(curr.start, "too less arguments given");
      }
      fn_scope.variables[_PROG.at(arg_name).as.IDENTIFIER.index] =
          interpret_expression(scope, arg);
      arg = _PROG.at(arg).next_child;
      arg_name = _PROG.at(arg_name).next_child;
    }
    if (!arg.is_null()) {
      throw msl_runtime_error(curr.start, "too many arguments given");
    }
  }
  node_idx fn_body = _PROG.at(fn.first_child).next_child;
  while (!fn_body.is_null()) {
    interpret_one(&fn_scope, fn_body);
    fn_body = _PROG.at(fn_body).next_child;
  }
  return fn_scope.return_value;
}

inline void interpret_if(Scope *scope, Node curr) {
  Node if_node = _PROG.at(curr.first_child);
  Value if_cond = interpret_expression(scope, if_node.first_child);
  bool condition_was_true = value_as_bool(if_cond);

  if (condition_was_true) {
    node_idx if_body = _PROG.at(if_node.first_child).next_child;
    while (!if_body.is_null()) {
      interpret_one(scope, if_body);
      if_body = _PROG.at(if_body).next_child;
    }
  }

  node_idx next_partial_condition = if_node.next_child;
  if (!condition_was_true) {
    Node cond_node = _PROG.at(next_partial_condition);
    while (!next_partial_condition.is_null() &&
           cond_node.tag == NodeTag::INTERNAL_PARTIAL_CONDITION) {
      Value cond = interpret_expression(scope, cond_node.first_child);
      bool cond_value = value_as_bool(cond);

      if (cond_value) {
        condition_was_true = true;
        node_idx cond_body = _PROG.at(cond_node.first_child).next_child;
        while (!cond_body.is_null()) {
          interpret_one(scope, cond_body);
          cond_body = _PROG.at(cond_body).next_child;
        }
      }
      next_partial_condition = cond_node.next_child;
      cond_node = _PROG.at(next_partial_condition);
    }
  }
  if (!condition_was_true) {
    if (!next_partial_condition.is_null() &&
        _PROG.at(next_partial_condition).tag ==
            NodeTag::INTERNAL_PARTIAL_DEFAULT_CONDITION) {
      node_idx cond_body = _PROG.at(next_partial_condition).first_child;
      while (!cond_body.is_null()) {
        interpret_one(scope, cond_body);
        cond_body = _PROG.at(cond_body).next_child;
      }
    }
  }
}

// ---------- EXECUTION LOGIC
Value interpret_expression(Scope *scope, node_idx node) {
  if (scope && scope->is_returning) {
    return Value::None();
  }
  Node curr = _PROG.at(node);
  switch (curr.tag) {
  case NodeTag::FN_CALL:
    return interpret_fn_call(scope, curr);
  case NodeTag::NIL:
    throw msl_runtime_error(curr.start,
                            "BUG: encountered null node of AST tree");
  case NodeTag::FN_DEF:
    throw msl_runtime_error(curr.start,
                            "BUG: encountered statement as expression");
  case NodeTag::VAR_DEF:
    throw msl_runtime_error(curr.start,
                            "BUG: encountered statement as expression");
  case NodeTag::VAR_SET:
    throw msl_runtime_error(curr.start,
                            "BUG: encountered statement as expression");
  case NodeTag::VAR_REF:
    if (scope && scope->has_var(curr.as.IDENTIFIER)) {
      return scope->get_var(curr.as.IDENTIFIER);
    } else {
      return _GLOBAL.variables.at(curr.as.IDENTIFIER.index);
    }
  case NodeTag::LITERAL_INT:
    return Value::Int(curr.as.INT);
  case NodeTag::LITERAL_STRING: {
    auto str = _HEAP.alloc_new_string(curr.as.STRING);
    return str;
  }
  case NodeTag::LITERAL_FLOAT:
    return Value::Float(curr.as.INT);
  case NodeTag::LITERAL_SYMBOL:
    return Value::Symbol(curr.as.SYMBOL);
  case NodeTag::PREFIX_NOT:
    return interpret_operator_not(scope, curr);
  case NodeTag::PREFIX_INVERS:
    return interpret_operator_invers(scope, curr);
  case NodeTag::INFIX_ADD:
    return interpret_operator_add(scope, curr);
  case NodeTag::INFIX_SUB:
    return interpret_operator_sub(scope, curr);
  case NodeTag::INFIX_MUL:
    return interpret_operator_mul(scope, curr);
  case NodeTag::INFIX_DIV:
    return interpret_operator_div(scope, curr);
  case NodeTag::INFIX_MOD:
    return interpret_operator_mod(scope, curr);
  case NodeTag::INFIX_AND:
    return interpret_operator_and(scope, curr);
  case NodeTag::INFIX_OR:
    return interpret_operator_or(scope, curr);
  case NodeTag::INFIX_LT:
    return interpret_operator_lt(scope, curr);
  case NodeTag::INFIX_LTE:
    return interpret_operator_lte(scope, curr);
  case NodeTag::INFIX_GT:
    return interpret_operator_gt(scope, curr);
  case NodeTag::INFIX_GTE:
    return interpret_operator_gte(scope, curr);
  case NodeTag::INFIX_EQ:
    return interpret_operator_eq(scope, curr);
  case NodeTag::INFIX_NEQ:
    return interpret_operator_neq(scope, curr);
  case NodeTag::INFIX_STR_CONCAT:
    return interpret_operator_str_concat(scope, curr);
  case NodeTag::IF:
    throw msl_runtime_error(curr.start,
                            "BUG: encountered statement as expression");
  case NodeTag::FUNCTION:
    throw msl_runtime_error(curr.start, "BUG: function definition "
                                        "statement encountered as expression");
  case NodeTag::RETURN:
    throw msl_runtime_error(curr.start,
                            "BUG: encountered statement as expression");
  case NodeTag::INTERNAL_PARTIAL_CONDITION:
  case NodeTag::INTERNAL_PARTIAL_DEFAULT_CONDITION:
    throw msl_runtime_error(
        curr.start,
        "BUG: partial condition statement encountered as expression");
  case NodeTag::INTERNAL_LIST:
    throw msl_runtime_error(
        curr.start, "BUG: encountered internal list node as expression");
  }
}

void interpret_one(Scope *scope, node_idx node) {
  if (scope && scope->is_returning) {
    return;
  }
  Node curr = _PROG.at(node);
  switch (curr.tag) {
  case NodeTag::NIL:
    throw msl_runtime_error(curr.start,
                            "BUG: encountered null node of AST tree");
  case NodeTag::FN_CALL:
    interpret_fn_call(scope, curr);
    break;
  case NodeTag::FN_DEF:
    _GLOBAL.functions[curr.as.IDENTIFIER.index] = curr.first_child;
    break;
  case NodeTag::VAR_DEF:
    if (scope && scope->variables.count(curr.as.IDENTIFIER.index) > 0) {
      throw msl_runtime_error(curr.start, "variable already defined");
    } else if (scope) {
      scope->variables[curr.as.IDENTIFIER.index] =
          interpret_expression(scope, curr.first_child);
    } else if (_GLOBAL.variables.count(curr.as.IDENTIFIER.index) > 0) {
      throw msl_runtime_error(curr.start, "variable already defined");
    } else {
      _GLOBAL.variables[curr.as.IDENTIFIER.index] =
          interpret_expression(scope, curr.first_child);
    }
  case NodeTag::VAR_SET: {
    if (scope && scope->has_var(curr.as.IDENTIFIER)) {
      scope->set_var(curr.as.IDENTIFIER,
                     interpret_expression(scope, curr.first_child));
    } else if (_GLOBAL.variables.count(curr.as.IDENTIFIER.index) > 0) {
      _GLOBAL.variables.at(curr.as.IDENTIFIER.index) =
          interpret_expression(scope, curr.first_child);
    } else {
      throw msl_runtime_error(curr.start, "undefined variable name");
    }
  } break;
  case NodeTag::VAR_REF:
    break;
  case NodeTag::LITERAL_INT:
    break;
  case NodeTag::LITERAL_STRING:
    break;
  case NodeTag::LITERAL_FLOAT:
    break;
  case NodeTag::LITERAL_SYMBOL:
    break;
  case NodeTag::PREFIX_NOT:
    interpret_operator_not(scope, curr);
    break;
  case NodeTag::PREFIX_INVERS:
    interpret_operator_invers(scope, curr);
    break;
  case NodeTag::INFIX_ADD:
    interpret_operator_add(scope, curr);
    break;
  case NodeTag::INFIX_SUB:
    interpret_operator_sub(scope, curr);
    break;
  case NodeTag::INFIX_MUL:
    interpret_operator_mul(scope, curr);
    break;
  case NodeTag::INFIX_DIV:
    interpret_operator_div(scope, curr);
    break;
  case NodeTag::INFIX_MOD:
    interpret_operator_mod(scope, curr);
    break;
  case NodeTag::INFIX_AND:
    interpret_operator_and(scope, curr);
    break;
  case NodeTag::INFIX_OR:
    interpret_operator_or(scope, curr);
    break;
  case NodeTag::INFIX_LT:
    interpret_operator_lt(scope, curr);
    break;
  case NodeTag::INFIX_LTE:
    interpret_operator_lte(scope, curr);
    break;
  case NodeTag::INFIX_GT:
    interpret_operator_gt(scope, curr);
    break;
  case NodeTag::INFIX_GTE:
    interpret_operator_gte(scope, curr);
    break;
  case NodeTag::INFIX_EQ:
    interpret_operator_eq(scope, curr);
    break;
  case NodeTag::INFIX_NEQ:
    interpret_operator_neq(scope, curr);
    break;
  case NodeTag::INFIX_STR_CONCAT:
    interpret_operator_str_concat(scope, curr);
    break;
  case NodeTag::IF:
    interpret_if(scope, curr);
    break;
  case NodeTag::FUNCTION: {
    if (scope) {
      throw msl_runtime_error(curr.start,
                              "function definition not allowed here");
    } else if (_GLOBAL.functions.count(curr.as.IDENTIFIER.index) > 0) {
      throw msl_runtime_error(curr.start, "function already defined");
    } else {
      _GLOBAL.functions[curr.as.IDENTIFIER.index] = node;
    }
    break;
  }
  case NodeTag::RETURN: {
    if (!scope) {
      throw msl_runtime_error(
          curr.start, "return is not allowed outside of a function scope");
    }
    Value return_value = interpret_expression(scope, curr.first_child);
    scope->is_returning = true;
    scope->return_value = return_value;
  } break;
  case NodeTag::INTERNAL_PARTIAL_CONDITION:
  case NodeTag::INTERNAL_PARTIAL_DEFAULT_CONDITION:
    throw msl_runtime_error(
        curr.start,
        "BUG: encountered internal partical condition node as statement");
  case NodeTag::INTERNAL_LIST:
    throw msl_runtime_error(curr.start,
                            "BUG: encountered internal list node as statement");
  }
}

void free_heap() {
  for (size_t i = 0; i < _HEAP.elements.size(); i++) {
    if (_HEAP.elements.at(i).tag == ValueTag::STRING) {
      delete _HEAP.elements.at(i).as.STRING;
    }
  }
}
}; // namespace

int interpret(nodes ns) {
  GlobalContext global;
  _GLOBAL = global;
  _PROG = ns;
  _HEAP = Heap();
  node_idx curr = node_idx{_PROG.first_elem};
  while (!curr.is_null()) {
    interpret_one(NULL, curr);
    curr = _PROG.at(curr).next_sibling;
  }
  free_heap();
  return 0;
}
