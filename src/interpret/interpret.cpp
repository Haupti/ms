#include "../../lib/asap/util.hpp"
#include "../interned_string.hpp"
#include "../msl_runtime_error.hpp"
#include "../parser/node.hpp"
#include "buildin/operators.hpp"
#include "constants.hpp"
#include "scope.hpp"
#include "value_and_heap.hpp"
#include <functional>
#include <unordered_map>

using namespace std;
namespace {
// ------- TYPES -------
// ------- TYPES -------
// ------- TYPES -------

struct GlobalContext {
  // key is InternedString unpacked
  unordered_map<uint64_t, node_idx> functions;
  unordered_map<uint64_t, Value> variables;
};

// ------- GLOBALS -------
// ------- GLOBALS -------
// ------- GLOBALS -------

static nodes _PROG;
static GlobalContext _GLOBAL;
static ILHeap _HEAP = ILHeap(10000, 1000);

// ------- LOGIC -------
// ------- LOGIC -------
// ------- LOGIC -------

// ------- UTILS -------
// ------- UTILS -------
// ------- UTILS -------
inline bool value_as_bool(const Value &value) {
  // TODO either that or throw if its not #true/#false not sure yet
  return value.tag == ValueTag::SYMBOL &&
         value.as.SYMBOL.index == _SYM_TRUE.index;
}
inline int64_t value_as_int(LocationRef location, const Value &value) {
  if (value.tag != ValueTag::INT) {
    throw msl_runtime_error(location, "expected a int value");
  }
  return value.as.INT;
}

inline void assert_list(LocationRef where, ValueTag tag) {
  if (tag != ValueTag::LIST) {
    throw msl_runtime_error(where, "expected a list");
  }
}

Value interpret_expression(Scope *scope, node_idx node);
// ------- BUILDIN FUNCTIONS -------
// ------- BUILDIN FUNCTIONS -------
// ------- BUILDIN FUNCTIONS -------
Value msl_buildin_append(Scope *scope, Node node) {
  const Node &_fn_name = _PROG.at(node.first_child);
  Value list_head = interpret_expression(scope, _fn_name.next_child);
  assert_list(node.start, list_head.tag);
  Node _list_var_ref_node = _PROG.at(_fn_name.next_child);
  const Value &value =
      interpret_expression(scope, _list_var_ref_node.next_child);
  auto res = _HEAP.add_child(list_head.as.LIST, value);
  if (res == 0) {
    throw msl_runtime_error(node.start, "failed to add element to list");
  }
  return list_head;
}
Value msl_buildin_prepend(Scope *scope, Node node) {
  const Node &_fn_name = _PROG.at(node.first_child);
  Value list_head = interpret_expression(scope, _fn_name.next_child);
  assert_list(node.start, list_head.tag);
  Node _list_var_ref_node = _PROG.at(_fn_name.next_child);
  const Value &value =
      interpret_expression(scope, _list_var_ref_node.next_child);
  auto res = _HEAP.add_child_front(list_head.as.LIST, value);
  if (res == 0) {
    throw msl_runtime_error(node.start, "failed to add element to list");
  }
  return list_head;
}
Value msl_buildin_concat(Scope *scope, Node node) {
  const Node &_fn_name = _PROG.at(node.first_child);
  Value list_head_left = interpret_expression(scope, _fn_name.next_child);
  assert_list(node.start, list_head_left.tag);
  Node _list_var_ref_node = _PROG.at(_fn_name.next_child);
  const Value &list_head_right =
      interpret_expression(scope, _list_var_ref_node.next_child);
  assert_list(node.start, list_head_right.tag);

  ILHIDX element = _HEAP.nth_child_idx(list_head_right.as.LIST, 0);
  uint64_t i = 0;
  while (element != INVALID) {
    auto res = _HEAP.add_child(list_head_left.as.LIST, _HEAP.at(element));
    if (res == 0) {
      throw msl_runtime_error(node.start, "failed to add element to list");
    }
    i++;
    element = _HEAP.nth_child_idx(list_head_right.as.LIST, i);
  }
  return list_head_left;
}
Value msl_buildin_list(Scope *scope, Node node) {
  Value list = Value::EmptyList();
  ILHIDX list_idx = _HEAP.add(list);
  list.as.LIST = list_idx;
  node_idx list_element = _PROG.next_child(node.first_child);
  while (!list_element.is_null()) {
    Value elem = interpret_expression(scope, list_element);
    _HEAP.add_child(list.as.LIST, elem);
    list_element = _PROG.next_child(list_element);
  }
  return list;
}
Value msl_buildin_put(Scope *scope, Node node) {
  const Node &_fn_name = _PROG.at(node.first_child);
  Value list_head = interpret_expression(scope, _fn_name.next_child);
  assert_list(node.start, list_head.tag);
  Node _list_var_ref_node = _PROG.at(_fn_name.next_child);
  const Value &val_index =
      interpret_expression(scope, _list_var_ref_node.next_child);
  int64_t index = value_as_int(_list_var_ref_node.start, val_index);
  Value value = interpret_expression(
      scope, _PROG.next_child(_list_var_ref_node.next_child));
  if (index < 0) {
    msl_runtime_error(_list_var_ref_node.start,
                      "index out of bounds: " + to_string(index));
  }

  ILHIDX list_elem_idx = _HEAP.nth_child_idx(list_head.as.LIST, index);
  _HEAP.replace(list_elem_idx, value);
  return Value();
}
Value msl_buildin_at(Scope *scope, Node node) {
  const Node &_fn_name = _PROG.at(node.first_child);
  Value list_head = interpret_expression(scope, _fn_name.next_child);
  assert_list(node.start, list_head.tag);
  Node _list_var_ref_node = _PROG.at(_fn_name.next_child);
  const Value &val_index =
      interpret_expression(scope, _list_var_ref_node.next_child);
  int64_t index = value_as_int(_list_var_ref_node.start, val_index);
  if (index < 0) {
    msl_runtime_error(_list_var_ref_node.start,
                      "index out of bounds: " + to_string(index));
  }
  return _HEAP.nth_child(list_head.as.LIST, index);
}

string value_to_string(const Value &value) {
  switch (value.tag) {
  case ValueTag::INT:
    return to_string(value.as.INT);
  case ValueTag::STRING:
    return _HEAP.get_string(value.as.STRING);
  case ValueTag::FLOAT:
    return to_string(value.as.FLOAT);
  case ValueTag::SYMBOL:
    return resolve_symbol(value.as.SYMBOL);
  case ValueTag::LIST: {
    vector<string> elems;
    uint64_t i = 0;
    while (0 != _HEAP.nth_child_idx(value.as.LIST, i)) {
      elems.push_back(value_to_string(_HEAP.nth_child(value.as.LIST, i)));
      i++;
    }
    return "[" + join(elems, ",") + "]";
  };
  case ValueTag::NONE:
    return "none";
  case ValueTag::ERROR:
    return "error(" + value_to_string(_HEAP.at(value.as.ERROR)) + ")";
    break;
  }
}
Value msl_buildin_println(Scope *scope, Node node) {
  Value arg = interpret_expression(scope, _PROG.nth_child(node, 1));
  println(value_to_string(arg));
  return Value();
}

struct BuildinFnMap {
  std::unordered_map<uint64_t, std::function<Value(Scope *, Node)>> buildins = {
      {_BUILDIN_FN_PRINT.index, msl_buildin_println},
      {_BUILDIN_FN_LIST.index, msl_buildin_list},
      {_BUILDIN_FN_PUT.index, msl_buildin_put},
      {_BUILDIN_FN_AT.index, msl_buildin_at},
      {_BUILDIN_FN_APPEND.index, msl_buildin_append},
      {_BUILDIN_FN_PREPEND.index, msl_buildin_prepend},
      {_BUILDIN_FN_CONCAT.index, msl_buildin_concat},
  };
  bool has(InternedString name) { return buildins.count(name.index) > 0; }
  Value call(Scope *scope, Node curr, InternedString name) {
    return buildins[name.index](scope, curr);
  }
};
// --------- OPERATORS

inline Value interpret_operator_str_concat(Scope *scope, Node curr) {
  Value leftval = interpret_expression(scope, curr.first_child);
  Value rightval =
      interpret_expression(scope, _PROG.at(curr.first_child).next_child);
  return mslbuildin_op_str_concat(&_HEAP, curr.start, leftval, rightval);
}

inline Value interpret_operator_neq(Scope *scope, Node curr) {
  Value leftval = interpret_expression(scope, curr.first_child);
  Value rightval =
      interpret_expression(scope, _PROG.at(curr.first_child).next_child);
  return mslbuildin_op_neq(&_HEAP, leftval, rightval);
}

inline Value interpret_operator_eq(Scope *scope, Node curr) {
  Value leftval = interpret_expression(scope, curr.first_child);
  Value rightval =
      interpret_expression(scope, _PROG.at(curr.first_child).next_child);
  return mslbuildin_op_eq(&_HEAP, leftval, rightval);
}

inline Value interpret_operator_gte(Scope *scope, Node curr) {
  Value leftval = interpret_expression(scope, _PROG.nth_child(curr, 0));
  Value rightval = interpret_expression(scope, _PROG.nth_child(curr, 1));
  return mslbuildin_op_gte(curr.start, leftval, rightval);
}

inline Value interpret_operator_gt(Scope *scope, Node curr) {
  Value leftval = interpret_expression(scope, _PROG.nth_child(curr, 0));
  Value rightval = interpret_expression(scope, _PROG.nth_child(curr, 1));
  return mslbuildin_op_gt(curr.start, leftval, rightval);
}

inline Value interpret_operator_lte(Scope *scope, Node curr) {
  Value leftval = interpret_expression(scope, _PROG.nth_child(curr, 0));
  Value rightval = interpret_expression(scope, _PROG.nth_child(curr, 1));
  return mslbuildin_op_lte(curr.start, leftval, rightval);
}

inline Value interpret_operator_lt(Scope *scope, Node curr) {
  Value leftval = interpret_expression(scope, _PROG.nth_child(curr, 0));
  Value rightval = interpret_expression(scope, _PROG.nth_child(curr, 1));
  return mslbuildin_op_lt(curr.start, leftval, rightval);
}

inline Value interpret_operator_mod(Scope *scope, Node curr) {
  Value leftval = interpret_expression(scope, _PROG.nth_child(curr, 0));
  Value rightval = interpret_expression(scope, _PROG.nth_child(curr, 1));
  return mslbuildin_op_mod(curr.start, leftval, rightval);
}
inline Value interpret_operator_div(Scope *scope, Node curr) {
  Value leftval = interpret_expression(scope, _PROG.nth_child(curr, 0));
  Value rightval = interpret_expression(scope, _PROG.nth_child(curr, 1));
  return mslbuildin_op_div(curr.start, leftval, rightval);
}
inline Value interpret_operator_mul(Scope *scope, Node curr) {
  Value leftval = interpret_expression(scope, _PROG.nth_child(curr, 0));
  Value rightval = interpret_expression(scope, _PROG.nth_child(curr, 1));
  return mslbuildin_op_mul(curr.start, leftval, rightval);
}

inline Value interpret_operator_sub(Scope *scope, Node curr) {
  Value leftval = interpret_expression(scope, _PROG.nth_child(curr, 0));
  Value rightval = interpret_expression(scope, _PROG.nth_child(curr, 1));
  return mslbuildin_op_sub(curr.start, leftval, rightval);
}

inline Value interpret_operator_add(Scope *scope, Node curr) {
  Value leftval = interpret_expression(scope, _PROG.nth_child(curr, 0));
  Value rightval = interpret_expression(scope, _PROG.nth_child(curr, 1));
  return mslbuildin_op_add(curr.start, leftval, rightval);
}

inline Value interpret_operator_invers(Scope *scope, Node curr) {
  Value val = interpret_expression(scope, _PROG.nth_child(curr, 0));
  return mslbuildin_op_invers(curr.start, val);
}

inline Value interpret_operator_or(Scope *scope, Node curr) {
  Value leftval = interpret_expression(scope, _PROG.nth_child(curr, 0));
  if (leftval.tag == ValueTag::SYMBOL &&
      leftval.as.SYMBOL.index == _SYM_TRUE.index) {
    return Value::Symbol(_SYM_TRUE);
  }
  Value rightval = interpret_expression(scope, _PROG.nth_child(curr, 1));
  if (rightval.tag == ValueTag::SYMBOL &&
      rightval.as.SYMBOL.index == _SYM_TRUE.index) {
    return Value::Symbol(_SYM_TRUE);
  } else {
    return Value::Symbol(_SYM_FALSE);
  }
}
inline Value interpret_operator_and(Scope *scope, Node curr) {
  Value leftval = interpret_expression(scope, _PROG.nth_child(curr, 0));
  if (leftval.tag == ValueTag::SYMBOL &&
      leftval.as.SYMBOL.index != _SYM_TRUE.index) {
    return Value::Symbol(_SYM_FALSE);
  } else {
    return Value::Symbol(_SYM_FALSE);
  }
  Value rightval = interpret_expression(scope, _PROG.nth_child(curr, 1));
  if (rightval.tag == ValueTag::SYMBOL &&
      rightval.as.SYMBOL.index == _SYM_TRUE.index) {
    return Value::Symbol(_SYM_TRUE);
  } else {
    return Value::Symbol(_SYM_FALSE);
  }
}

inline Value interpret_operator_not(Scope *scope, Node curr) {
  Value val = interpret_expression(scope, _PROG.nth_child(curr, 0));
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

// ---------- MAIN LOGIC
void interpret_one(Scope *scope, node_idx node);

Value interpret_fn_call(Scope *scope, Node curr) {
  static BuildinFnMap buildins;
  InternedString fn_name = _PROG.nth_child_node(curr, 0).as.IDENTIFIER;

  if (buildins.has(fn_name)) {
    return buildins.call(scope, curr, fn_name);
  }

  Scope fn_scope;
  fn_scope.parent = scope;
  node_idx function = _GLOBAL.functions.at(fn_name.index);
  Node fn = _PROG.at(function);

  // first element is a list of args, first list element is first arg
  node_idx arg_name = _PROG.first_child(fn.first_child);
  node_idx arg = _PROG.nth_child(curr, 1);
  if (!arg_name.is_null()) {
    while (!arg_name.is_null()) {
      if (arg.is_null()) {
        throw msl_runtime_error(curr.start, "too less arguments given");
      }
      fn_scope.variables[_PROG.at(arg_name).as.IDENTIFIER.index] =
          interpret_expression(scope, arg);
      arg = _PROG.next_child(arg);
      arg_name = _PROG.next_child(arg_name);
    }
    if (!arg.is_null()) {
      throw msl_runtime_error(curr.start, "too many arguments given");
    }
  }
  node_idx fn_body = _PROG.nth_child(fn, 1);
  while (!fn_body.is_null()) {
    interpret_one(&fn_scope, fn_body);
    fn_body = _PROG.next_child(fn_body);
  }
  return fn_scope.return_value;
}

// TODO currently there is no new scope created for the if/elif/else body
// evaluation.
// this should be done, also then!!! there must be a mechanism that passes the
// returning state one up. maybe that would be a good time to implement a more
// 'flat' approach to scopes rather than hirachy. maybe that would also be a
// good time to realize that converting the AST to a absolute positioned
// instruction vector would be a good idea.
// ... yeah much like that of a VM, that apparently makes some things a LOT
// easier. also GC would be a lot easier that way since i dont have to traverse
// a hirachy of scopes that way...
inline void interpret_if(Scope *scope, Node curr) {
  Node if_node = _PROG.at(curr.first_child);
  Value if_cond = interpret_expression(scope, if_node.first_child);
  bool condition_was_true = value_as_bool(if_cond);

  if (condition_was_true) {
    node_idx if_body = _PROG.nth_child(if_node, 1);
    while (!if_body.is_null()) {
      interpret_one(scope, if_body);
      if_body = _PROG.next_sibling(if_body);
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
        node_idx cond_body = _PROG.nth_child(cond_node, 1);
        while (!cond_body.is_null()) {
          interpret_one(scope, cond_body);
          cond_body = _PROG.first_child(cond_body);
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
      node_idx cond_body = _PROG.first_child(next_partial_condition);
      while (!cond_body.is_null()) {
        interpret_one(scope, cond_body);
        cond_body = _PROG.next_child(cond_body);
      }
    }
  }
}

inline Value interpret_try(Scope *, Node) {
  throw runtime_error("NOT YET IMPLEMENTED");
  return Value();
}

inline Value interpret_expect(Scope *, Node) {
  throw runtime_error("NOT YET IMPLEMENTED");
  return Value();
}

// ---------- EXECUTION LOGIC
Value interpret_expression(Scope *scope, node_idx node) {
  if (scope && scope->is_returning) {
    return Value();
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
    } else if (_GLOBAL.variables.count(curr.as.IDENTIFIER.index) == 0) {
      throw msl_runtime_error(
          curr.start, "undefined variable '" +
                          resolve_interned_string(curr.as.IDENTIFIER) + "'");
    } else {
      return _GLOBAL.variables.at(curr.as.IDENTIFIER.index);
    }
  case NodeTag::LITERAL_INT:
    return Value::Int(curr.as.INT);
  case NodeTag::LITERAL_STRING: {
    ILHIDX str_idx = _HEAP.add_string(curr.as.STRING);
    return _HEAP.at(str_idx);
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
  case NodeTag::TRY:
    return interpret_try(scope, curr);
  case NodeTag::EXPECT:
    return interpret_expect(scope, curr);
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
  case NodeTag::FN_DEF: {
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
  case NodeTag::TRY:
    interpret_try(scope, curr);
    break;
  case NodeTag::EXPECT:
    interpret_expect(scope, curr);
    break;
  }
}
}; // namespace

int interpret(nodes ns) {
  GlobalContext global;
  _GLOBAL = global;
  _PROG = ns;
  node_idx curr = node_idx{_PROG.first_elem};
  while (!curr.is_null()) {
    interpret_one(NULL, curr);
    curr = _PROG.next_sibling(curr);
  }
  return 0;
}
