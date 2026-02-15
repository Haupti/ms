#include "../../lib/asap/util.hpp"
#include "../../tests/testutil_node_to_string.hpp"
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
  Value alloc_new_string(string str) {
    HeapNode node = {};
    node.as.STRING = new string(str);
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
    static_strings[str.index] = new_string_idx;
    return value;
  }
  string heap_string(HeapNodeIdx idx) {
    string aaa = *elements.at(idx.idx).as.STRING;
    return aaa;
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

static nodes _PROG;
static GlobalContext _GLOBAL;
static Heap _HEAP;
static Symbol _SYM_TRUE = create_symbol("#true");
static Symbol _SYM_FALSE = create_symbol("#false");
static InternedString _BUILDIN_FN_PRINT = create_interned_string("print");

void interpret_one(Scope *scope, node_idx node);
Value interpret_expression(Scope *scope, node_idx node) {
  if (scope && scope->is_returning) {
    return Value::None();
  }
  Node curr = _PROG.get(node);
  switch (curr.tag) {
  case NodeTag::FN_CALL: {
    InternedString fn_name = _PROG.get(curr.first_child).as.IDENTIFIER;
    if (fn_name.index == _BUILDIN_FN_PRINT.index) {
      Value arg =
          interpret_expression(scope, _PROG.get(curr.first_child).next_child);
      switch (arg.tag) {
      case ValueTag::INT:
        println(to_string(arg.as.INT));
        break;
      case ValueTag::STRING:
        println(_HEAP.heap_string(arg.as.STRING));
        break;
      case ValueTag::FLOAT:
        println(to_string(arg.as.FLOAT));
        break;
      case ValueTag::SYMBOL:
        println(resolve_symbol(arg.as.SYMBOL));
        break;
      case ValueTag::LIST:
        throw runtime_error("NOT YET IMPLEMENTED");
        break;
      case ValueTag::NONE:
        println("none");
        break;
      }
      return Value::None();
    }
    Scope fn_scope;
    fn_scope.parent = scope;
    node_idx function = _GLOBAL.functions.at(fn_name.index);
    Node fn = _PROG.get(function);
    node_idx arg_name = _PROG.get(fn.first_child).first_child;
    node_idx arg = _PROG.get(curr.first_child).next_child;
    if (!arg_name.is_null()) {
      while (!arg_name.is_null()) {
        if (arg.is_null()) {
          throw msl_runtime_error(curr.start, "too less arguments given");
        }
        fn_scope.variables.at(_PROG.get(arg_name).as.IDENTIFIER.index) =
            interpret_expression(scope, arg);
        arg = _PROG.get(arg).next_child;
        arg_name = _PROG.get(arg_name).next_child;
      }
      if (!arg.is_null()) {
        throw msl_runtime_error(curr.start, "too many arguments given");
      }
    }
    node_idx fn_body = _PROG.get(fn.first_child).next_child;
    while (!fn_body.is_null()) {
      interpret_one(&fn_scope, fn_body);
      fn_body = _PROG.get(fn_body).next_child;
    }
    return fn_scope.return_value;
  } break;
  case NodeTag::NIL:
    throw msl_runtime_error(curr.start,
                            "this is a bug: encountered null node of AST tree");
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
  case NodeTag::PREFIX_NOT: {
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
  case NodeTag::PREFIX_INVERS: {
    Value val = interpret_expression(scope, curr.first_child);
    switch (val.tag) {
    case ValueTag::INT:
      return Value::Int(-1 * val.as.INT);
    case ValueTag::FLOAT:
      return Value::Float(-1 * val.as.FLOAT);
    default:
      throw msl_runtime_error(curr.start, "'-' expected a number");
    }
  } break;
  case NodeTag::INFIX_ADD: {
    Value leftval = interpret_expression(scope, curr.first_child);
    Value rightval =
        interpret_expression(scope, _PROG.get(curr.first_child).next_child);
    switch (leftval.tag) {
    case ValueTag::INT: {
      switch (rightval.tag) {
      case ValueTag::INT:
        return Value::Int(leftval.as.INT + rightval.as.INT);
      case ValueTag::FLOAT:
        return Value::Float(leftval.as.INT + rightval.as.FLOAT);
      default:
        throw msl_runtime_error(curr.start,
                                "'+' expected two number arguments");
      }
    } break;

    case ValueTag::FLOAT: {
      switch (rightval.tag) {
      case ValueTag::INT:
        return Value::Float(leftval.as.FLOAT + rightval.as.INT);
      case ValueTag::FLOAT:
        return Value::Float(leftval.as.FLOAT + rightval.as.FLOAT);
      default:
        throw msl_runtime_error(curr.start,
                                "'+' expected two number arguments");
      }
    } break;
    default:
      throw msl_runtime_error(curr.start, "'+' expected two number arguments");
    }
  } break;

  case NodeTag::INFIX_SUB: {
    Value leftval = interpret_expression(scope, curr.first_child);
    Value rightval =
        interpret_expression(scope, _PROG.get(curr.first_child).next_child);
    switch (leftval.tag) {
    case ValueTag::INT: {
      switch (rightval.tag) {
      case ValueTag::INT:
        return Value::Int(leftval.as.INT - rightval.as.INT);
      case ValueTag::FLOAT:
        return Value::Float(leftval.as.INT - rightval.as.FLOAT);
      default:
        throw msl_runtime_error(curr.start,
                                "'-' expected two number arguments");
      }
    } break;

    case ValueTag::FLOAT: {
      switch (rightval.tag) {
      case ValueTag::INT:
        return Value::Float(leftval.as.FLOAT - rightval.as.INT);
      case ValueTag::FLOAT:
        return Value::Float(leftval.as.FLOAT - rightval.as.FLOAT);
      default:
        throw msl_runtime_error(curr.start,
                                "'-' expected two number arguments");
      }
    } break;
    default:
      throw msl_runtime_error(curr.start, "'-' expected two number arguments");
    }
  } break;

  case NodeTag::INFIX_MUL: {
    Value leftval = interpret_expression(scope, curr.first_child);
    Value rightval =
        interpret_expression(scope, _PROG.get(curr.first_child).next_child);
    switch (leftval.tag) {
    case ValueTag::INT: {
      switch (rightval.tag) {
      case ValueTag::INT:
        return Value::Int(leftval.as.INT * rightval.as.INT);
      case ValueTag::FLOAT:
        return Value::Float(leftval.as.INT * rightval.as.FLOAT);
      default:
        throw msl_runtime_error(curr.start,
                                "'*' expected two number arguments");
      }
    } break;

    case ValueTag::FLOAT: {
      switch (rightval.tag) {
      case ValueTag::INT:
        return Value::Float(leftval.as.FLOAT * rightval.as.INT);
      case ValueTag::FLOAT:
        return Value::Float(leftval.as.FLOAT * rightval.as.FLOAT);
      default:
        throw msl_runtime_error(curr.start,
                                "'*' expected two number arguments");
      }
    } break;
    default:
      throw msl_runtime_error(curr.start, "'*' expected two number arguments");
    }
  } break;

  case NodeTag::INFIX_DIV: {
    Value leftval = interpret_expression(scope, curr.first_child);
    Value rightval =
        interpret_expression(scope, _PROG.get(curr.first_child).next_child);
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
        throw msl_runtime_error(curr.start,
                                "'/' expected two number arguments");
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
        throw msl_runtime_error(curr.start,
                                "'/' expected two number arguments");
      }
    } break;
    default:
      throw msl_runtime_error(curr.start, "'/' expected two number arguments");
    }
  } break;

  case NodeTag::INFIX_MOD: {
    Value leftval = interpret_expression(scope, curr.first_child);
    Value rightval =
        interpret_expression(scope, _PROG.get(curr.first_child).next_child);
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
      throw msl_runtime_error(curr.start,
                              "'mod' expected two number arguments");
    }
  } break;

  case NodeTag::INFIX_AND: {
    Value leftval = interpret_expression(scope, curr.first_child);
    if (leftval.tag == ValueTag::SYMBOL &&
        leftval.as.SYMBOL.index != _SYM_TRUE.index) {
      return Value::Symbol(_SYM_FALSE);
    } else {
      return Value::Symbol(_SYM_FALSE);
    }
    Value rightval =
        interpret_expression(scope, _PROG.get(curr.first_child).next_child);
    if (rightval.tag == ValueTag::SYMBOL &&
        rightval.as.SYMBOL.index == _SYM_TRUE.index) {
      return Value::Symbol(_SYM_TRUE);
    } else {
      return Value::Symbol(_SYM_FALSE);
    }
  } break;
  case NodeTag::INFIX_OR: {
    Value leftval = interpret_expression(scope, curr.first_child);
    if (leftval.tag == ValueTag::SYMBOL &&
        leftval.as.SYMBOL.index == _SYM_TRUE.index) {
      return Value::Symbol(_SYM_TRUE);
    }
    Value rightval =
        interpret_expression(scope, _PROG.get(curr.first_child).next_child);
    if (rightval.tag == ValueTag::SYMBOL &&
        rightval.as.SYMBOL.index == _SYM_TRUE.index) {
      return Value::Symbol(_SYM_TRUE);
    } else {
      return Value::Symbol(_SYM_FALSE);
    }
  } break;

  case NodeTag::INFIX_LT: {
    Value leftval = interpret_expression(scope, curr.first_child);
    Value rightval =
        interpret_expression(scope, _PROG.get(curr.first_child).next_child);
    switch (leftval.tag) {
    case ValueTag::INT: {
      switch (rightval.tag) {
      case ValueTag::INT:
        return Value::Int(leftval.as.INT < rightval.as.INT);
      case ValueTag::FLOAT:
        return Value::Float(leftval.as.INT < rightval.as.FLOAT);
      default:
        throw msl_runtime_error(curr.start,
                                "'<' expected two number arguments");
      }
    } break;

    case ValueTag::FLOAT: {
      switch (rightval.tag) {
      case ValueTag::INT:
        return Value::Float(leftval.as.FLOAT < rightval.as.INT);
      case ValueTag::FLOAT:
        return Value::Float(leftval.as.FLOAT < rightval.as.FLOAT);
      default:
        throw msl_runtime_error(curr.start,
                                "'<' expected two number arguments");
      }
    } break;
    default:
      throw msl_runtime_error(curr.start, "'<' expected two number arguments");
    }
  } break;

  case NodeTag::INFIX_LTE: {
    Value leftval = interpret_expression(scope, curr.first_child);
    Value rightval =
        interpret_expression(scope, _PROG.get(curr.first_child).next_child);
    switch (leftval.tag) {
    case ValueTag::INT: {
      switch (rightval.tag) {
      case ValueTag::INT:
        return Value::Int(leftval.as.INT <= rightval.as.INT);
      case ValueTag::FLOAT:
        return Value::Float(leftval.as.INT <= rightval.as.FLOAT);
      default:
        throw msl_runtime_error(curr.start,
                                "'<=' expected two number arguments");
      }
    } break;

    case ValueTag::FLOAT: {
      switch (rightval.tag) {
      case ValueTag::INT:
        return Value::Float(leftval.as.FLOAT <= rightval.as.INT);
      case ValueTag::FLOAT:
        return Value::Float(leftval.as.FLOAT <= rightval.as.FLOAT);
      default:
        throw msl_runtime_error(curr.start,
                                "'<=' expected two number arguments");
      }
    } break;
    default:
      throw msl_runtime_error(curr.start, "'<=' expected two number arguments");
    }
  } break;
  case NodeTag::INFIX_GT: {
    Value leftval = interpret_expression(scope, curr.first_child);
    Value rightval =
        interpret_expression(scope, _PROG.get(curr.first_child).next_child);
    switch (leftval.tag) {
    case ValueTag::INT: {
      switch (rightval.tag) {
      case ValueTag::INT:
        return Value::Int(leftval.as.INT > rightval.as.INT);
      case ValueTag::FLOAT:
        return Value::Float(leftval.as.INT > rightval.as.FLOAT);
      default:
        throw msl_runtime_error(curr.start,
                                "'>' expected two number arguments");
      }
    } break;

    case ValueTag::FLOAT: {
      switch (rightval.tag) {
      case ValueTag::INT:
        return Value::Float(leftval.as.FLOAT > rightval.as.INT);
      case ValueTag::FLOAT:
        return Value::Float(leftval.as.FLOAT > rightval.as.FLOAT);
      default:
        throw msl_runtime_error(curr.start,
                                "'>' expected two number arguments");
      }
    } break;
    default:
      throw msl_runtime_error(curr.start, "'>' expected two number arguments");
    }
  } break;
  case NodeTag::INFIX_GTE: {
    Value leftval = interpret_expression(scope, curr.first_child);
    Value rightval =
        interpret_expression(scope, _PROG.get(curr.first_child).next_child);
    switch (leftval.tag) {
    case ValueTag::INT: {
      switch (rightval.tag) {
      case ValueTag::INT:
        return Value::Int(leftval.as.INT >= rightval.as.INT);
      case ValueTag::FLOAT:
        return Value::Float(leftval.as.INT >= rightval.as.FLOAT);
      default:
        throw msl_runtime_error(curr.start,
                                "'>=' expected two number arguments");
      }
    } break;

    case ValueTag::FLOAT: {
      switch (rightval.tag) {
      case ValueTag::INT:
        return Value::Float(leftval.as.FLOAT >= rightval.as.INT);
      case ValueTag::FLOAT:
        return Value::Float(leftval.as.FLOAT >= rightval.as.FLOAT);
      default:
        throw msl_runtime_error(curr.start,
                                "'>=' expected two number arguments");
      }
    } break;
    default:
      throw msl_runtime_error(curr.start, "'>' expected two number arguments");
    }
  } break;
  case NodeTag::INFIX_EQ: {
    Value leftval = interpret_expression(scope, curr.first_child);
    Value rightval =
        interpret_expression(scope, _PROG.get(curr.first_child).next_child);
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
  } break;

  case NodeTag::INFIX_NEQ: {
    Value leftval = interpret_expression(scope, curr.first_child);
    Value rightval =
        interpret_expression(scope, _PROG.get(curr.first_child).next_child);
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
  } break;
  case NodeTag::INFIX_STR_CONCAT: {
    Value leftval = interpret_expression(scope, curr.first_child);
    Value rightval =
        interpret_expression(scope, _PROG.get(curr.first_child).next_child);
    return _HEAP.alloc_new_string(_HEAP.heap_string(leftval.as.STRING) +
                                  _HEAP.heap_string(rightval.as.STRING));
  } break;
  case NodeTag::IF:
    throw runtime_error("NOT YET IMPLEMENTED");
    break;
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
  if (scope && scope->is_returning) {
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
    interpret_expression(scope, node);
    break;
  case NodeTag::PREFIX_INVERS:
    interpret_expression(scope, node);
    break;
  case NodeTag::INFIX_ADD:
    interpret_expression(scope, node);
    break;
  case NodeTag::INFIX_SUB:
    interpret_expression(scope, node);
    break;
  case NodeTag::INFIX_MUL:
    interpret_expression(scope, node);
    break;
  case NodeTag::INFIX_DIV:
    interpret_expression(scope, node);
    break;
  case NodeTag::INFIX_MOD:
    interpret_expression(scope, node);
    break;
  case NodeTag::INFIX_AND:
    interpret_expression(scope, node);
    break;
  case NodeTag::INFIX_OR:
    interpret_expression(scope, node);
    break;
  case NodeTag::INFIX_LT:
    interpret_expression(scope, node);
    break;
  case NodeTag::INFIX_LTE:
    interpret_expression(scope, node);
    break;
  case NodeTag::INFIX_GT:
    interpret_expression(scope, node);
    break;
  case NodeTag::INFIX_GTE:
    interpret_expression(scope, node);
    break;
  case NodeTag::INFIX_EQ:
    interpret_expression(scope, node);
    break;
  case NodeTag::INFIX_NEQ:
    interpret_expression(scope, node);
    break;
  case NodeTag::INFIX_STR_CONCAT:
    interpret_expression(scope, node);
    break;
  case NodeTag::IF:
    throw runtime_error("NOT YET IMPLEMENTED");
  case NodeTag::AT:
    interpret_expression(scope, node);
    break;
  case NodeTag::PUT:
    interpret_expression(scope, node);
    break;
  case NodeTag::PARTIAL_CONDITION:
  case NodeTag::PARTIAL_DEFAULT_CONDITION:
    throw msl_runtime_error(curr.start, "BUG: partial conditional encountered");
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
  case NodeTag::LIST:
    break;
  case NodeTag::RETURN:
    throw runtime_error("NOT YET IMPLEMENTED");
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
    curr = _PROG.get(curr).next_sibling;
  }
  free_heap();
  return 0;
}
