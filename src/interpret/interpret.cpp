#include "../msl_runtime_error.hpp"
#include "../parser/node.hpp"
#include <unordered_map>

using namespace std;
namespace {

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
    HeapNodeIdx new_string_idx;
    if (free_list.is_null()) {
      elements.push_back(node);
      new_string_idx = HeapNodeIdx(elements.size() - 1);
    } else {
      HeapNodeIdx current_free = free_list;
      free_list = elements.at(free_list.idx).next_sibling;
      elements[current_free.idx] = node;
      new_string_idx = current_free;
    }
    Value value;
    value.as.STRING = new_string_idx;
    value.tag = ValueTag::STRING;
    static_strings.at(str.index) = new_string_idx;
    return value;
  }
  Heap() { elements.reserve(10000); }

private:
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
  bool is_returning = false;
};

static nodes _PROG;
static GlobalContext _GLOBAL;
static Heap _HEAP;

void interpret_one(Scope *scope, node_idx node);
Value interpret_expression(Scope *scope, node_idx node) {
  if (scope->is_returning) {
    return Value::None();
  }
  Node curr = _PROG.get(node);
  switch (curr.tag) {
  case NodeTag::NIL:
    throw msl_runtime_error(curr.start,
                            "this is a bug: encountered null node of AST tree");
  case NodeTag::FN_CALL: {
    Scope fn_scope;
    node_idx function =
        _GLOBAL.functions.at(_PROG.get(curr.first_child).as.IDENTIFIER.index);
    Node fn = _PROG.get(function);
    node_idx arg_name = _PROG.get(fn.first_child).first_child;
    node_idx arg = _PROG.get(curr.first_child).next_child;
    if (!arg_name.is_null()) {
      while (!arg_name.is_null()) {
        fn_scope.variables.at(_PROG.get(arg_name).as.IDENTIFIER.index) =
            interpret_expression(scope, arg);
        arg = _PROG.get(arg).next_child;
        arg_name = _PROG.get(arg_name).next_child;
      }
    }
    node_idx fn_body = _PROG.get(fn.first_child).next_child;
    while (!fn_body.is_null()) {
      interpret_one(&fn_scope, fn_body);
      fn_body = _PROG.get(fn_body).next_child;
    }
    return fn_scope.return_value;
  } break;

  case NodeTag::FN_DEF:
    throw msl_runtime_error(
        curr.start, "this is a bug: encountered statement as expression");
  case NodeTag::VAR_DEF:
    throw msl_runtime_error(
        curr.start, "this is a bug: encountered statement as expression");
  case NodeTag::VAR_SET:
    throw msl_runtime_error(
        curr.start, "this is a bug: encountered statement as expression");
  case NodeTag::VAR_REF:
    if (scope) {
      // TODO either implement a way to access parent scopes if the variable is
      // not defined here (short hand solution)
      // OR during the validation rewire variable names to acces the one large
      // array stack frame thing basically also a LAOTs stack memory
      return scope->variables.at(curr.as.IDENTIFIER.index);
    } else {
      return _GLOBAL.variables.at(curr.as.IDENTIFIER.index);
    }
  case NodeTag::LITERAL_INT:
    return Value::Int(curr.as.INT);
  case NodeTag::LITERAL_STRING:
    return _HEAP.alloc_new_string(curr.as.STRING);
  case NodeTag::LITERAL_FLOAT:
    return Value::Float(curr.as.INT);
  case NodeTag::LITERAL_SYMBOL:
    return Value::Symbol(curr.as.SYMBOL);
  case NodeTag::PREFIX_NOT:
  case NodeTag::PREFIX_INVERS:
  case NodeTag::INFIX_ADD:
  case NodeTag::INFIX_SUB:
  case NodeTag::INFIX_MUL:
  case NodeTag::INFIX_DIV:
  case NodeTag::INFIX_MOD:
  case NodeTag::INFIX_AND:
  case NodeTag::INFIX_OR:
  case NodeTag::INFIX_LT:
  case NodeTag::INFIX_LTE:
  case NodeTag::INFIX_GT:
  case NodeTag::INFIX_GTE:
  case NodeTag::INFIX_EQ:
  case NodeTag::INFIX_NEQ:
  case NodeTag::INFIX_STR_CONCAT:
  case NodeTag::IF:
  case NodeTag::AT:
  case NodeTag::PUT:
  case NodeTag::PARTIAL_CONDITION:
  case NodeTag::PARTIAL_DEFAULT_CONDITION:
  case NodeTag::FUNCTION:
  case NodeTag::LIST:
    throw runtime_error("NOT YET IMPLEMENTED");
    break;
  case NodeTag::RETURN:
    throw msl_runtime_error(
        curr.start, "this is a bug: encountered statement as expression");
  }
}

void interpret_one(Scope *scope, node_idx node) {
  if (scope->is_returning) {
    return;
  }
  Node curr = _PROG.get(node);
  switch (curr.tag) {
  case NodeTag::NIL:
    throw msl_runtime_error(curr.start,
                            "this is a bug: encountered null node of AST tree");
  case NodeTag::FN_CALL:
    interpret_expression(scope, node);
    break;
  case NodeTag::FN_DEF:
    _GLOBAL.functions[curr.as.IDENTIFIER.index] = curr.first_child;
    break;
  case NodeTag::VAR_DEF:
    if (scope) {
      scope->variables[curr.as.IDENTIFIER.index] =
          interpret_expression(scope, curr.first_child);
    } else {
      _GLOBAL.variables[curr.as.IDENTIFIER.index] =
          interpret_expression(scope, curr.first_child);
    }
  case NodeTag::VAR_SET:
    if (scope) {
      scope->variables[curr.as.IDENTIFIER.index] =
          interpret_expression(scope, curr.first_child);
    } else {
      _GLOBAL.variables[curr.as.IDENTIFIER.index] =
          interpret_expression(scope, curr.first_child);
    }
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
  case NodeTag::PREFIX_INVERS:
  case NodeTag::INFIX_ADD:
  case NodeTag::INFIX_SUB:
  case NodeTag::INFIX_MUL:
  case NodeTag::INFIX_DIV:
  case NodeTag::INFIX_MOD:
  case NodeTag::INFIX_AND:
  case NodeTag::INFIX_OR:
  case NodeTag::INFIX_LT:
  case NodeTag::INFIX_LTE:
  case NodeTag::INFIX_GT:
  case NodeTag::INFIX_GTE:
  case NodeTag::INFIX_EQ:
  case NodeTag::INFIX_NEQ:
  case NodeTag::INFIX_STR_CONCAT:
  case NodeTag::IF:
  case NodeTag::AT:
  case NodeTag::PUT:
  case NodeTag::PARTIAL_CONDITION:
  case NodeTag::PARTIAL_DEFAULT_CONDITION:
  case NodeTag::FUNCTION:
  case NodeTag::LIST:
  case NodeTag::RETURN:
    break;
  }
}
}; // namespace

int interpret(nodes ns) {

  GlobalContext global;
  _GLOBAL = global;
  _PROG = ns;
  _HEAP = Heap();
  node_idx curr = node_idx(_PROG.first_elem);
  while (!curr.is_null()) {
    interpret_one(NULL, node_idx(_PROG.first_elem));
    curr = _PROG.get(curr).next_sibling;
  }
  return 0;
}
