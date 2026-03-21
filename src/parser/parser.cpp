#include "../compile_error.hpp"
#include "../preprocessor/token.hpp"
#include "node.hpp"
#include <vector>

using namespace std;

namespace {
struct Parser {
  vector<Token> tokens;
  nodes nodes;
  uint64_t pos;
  uint64_t len;
  bool eof() { return pos >= len; }
  void adv() { pos++; }
  Token peek() {
    if (pos >= len) {
      throw compile_error(tokens.back().location, "unexpected EOF");
    }
    return tokens.at(pos);
  }
};

#ifdef PRODUCTION
void assert_identifier(const Token &t) {
  if (t.tag != TokenTag::IDENTIFIER) {
    throw compile_error(t.location, "expected an identifier");
  }
}
void assert_in(const Token &t) {
  if (t.tag != TokenTag::IN) {
    throw compile_error(t.location, "expected 'in'");
  }
}
void assert_assign(const Token &t) {
  if (t.tag != TokenTag::ASSIGN) {
    throw compile_error(t.location, "expected '='");
  }
}
void assert_close_br(const Token &t) {
  if (t.tag != TokenTag::BRCLOSE) {
    throw compile_error(t.location, "expected ')'");
  }
}
void assert_close_bracket(const Token &t) {
  if (t.tag != TokenTag::BRACKETCLOSE) {
    throw compile_error(t.location, "expected ']'");
  }
}
void assert_open_br(const Token &t) {
  if (t.tag != TokenTag::BROPEN) {
    throw compile_error(t.location, "expected '('");
  }
}
void assert_open_cur(const Token &t) {
  if (t.tag != TokenTag::BROPEN) {
    throw compile_error(t.location, "expected '{'");
  }
}
#else
#define assert_identifier(token) __assert_identifier(token, __FILE__, __LINE__)
void __assert_identifier(Token t, const string &file, int line) {
  if (t.tag != TokenTag::IDENTIFIER) {
    throw __compile_error_debug(t.location, "expected an identifier", line,
                                file);
  }
}
#define assert_in(token) __assert_in(token, __FILE__, __LINE__)
void __assert_in(Token t, const string &file, int line) {
  if (t.tag != TokenTag::IN) {
    throw __compile_error_debug(t.location, "expected 'in'", line, file);
  }
}
#define assert_assign(token) __assert_assign(token, __FILE__, __LINE__)
void __assert_assign(Token t, const string &file, int line) {
  if (t.tag != TokenTag::ASSIGN) {
    throw __compile_error_debug(t.location, "expected '='", line, file);
  }
}
#define assert_close_br(token) __assert_close_br(token, __FILE__, __LINE__)
void __assert_close_br(Token t, const string &file, int line) {
  if (t.tag != TokenTag::BRCLOSE) {
    throw __compile_error_debug(t.location, "expected ')'", line, file);
  }
}
#define assert_close_bracket(token)                                            \
  __assert_close_bracket(token, __FILE__, __LINE__)
void __assert_close_bracket(Token t, const string &file, int line) {
  if (t.tag != TokenTag::BRACKETCLOSE) {
    throw __compile_error_debug(t.location, "expected ']'", line, file);
  }
}
#define assert_open_br(token) __assert_open_br(token, __FILE__, __LINE__)
void __assert_open_br(Token t, const string &file, int line) {
  if (t.tag != TokenTag::BROPEN) {
    throw __compile_error_debug(t.location, "expected '('", line, file);
  }
}
#define assert_open_cur(token) __assert_open_cur(token, __FILE__, __LINE__)
void __assert_open_cur(Token t, const string &file, int line) {
  if (t.tag != TokenTag::CURLOPEN) {
    throw __compile_error_debug(t.location, "expected '{'", line, file);
  }
}
#endif

node_idx parse_one(Parser *p);
node_idx parse_consecutive_expression(Parser *p, node_idx left,
                                      NodeTag left_tag,
                                      LocationRef left_location);
node_idx parse_expression_lazy(Parser *p);
node_idx parse_expression_eager(Parser *p) {
  node_idx left = parse_expression_lazy(p);
  Node left_node = p->nodes.at(left);
  return parse_consecutive_expression(p, left, left_node.tag, left_node.start);
}

node_idx parse_try_lazy(Parser *p) {
  LocationRef start = p->peek().location;
  p->adv();

  Node myself = Node();
  myself.start = start;
  myself.tag = NodeTag::TRY;
  node_idx myself_idx = p->nodes.add_dangling(myself);

  node_idx arg_idx = parse_expression_lazy(p);
  p->nodes.add_child(myself_idx, arg_idx);

  return myself_idx;
}

node_idx parse_expect_lazy(Parser *p) {
  LocationRef start = p->peek().location;
  p->adv();

  Node myself = Node();
  myself.start = start;
  myself.tag = NodeTag::EXPECT;
  node_idx myself_idx = p->nodes.add_dangling(myself);

  node_idx arg_idx = parse_expression_lazy(p);
  p->nodes.add_child(myself_idx, arg_idx);

  return myself_idx;
}

void parse_add_function_args(Parser *p, node_idx myself_idx) {
  while (true) {
    node_idx arg_idx = parse_expression_eager(p);
    p->nodes.add_child(myself_idx, arg_idx);
    Token next = p->peek();
    if (next.tag == TokenTag::COMMA) {
      p->adv();
    } else if (next.tag == TokenTag::BRCLOSE) {
      p->adv();
      break;
    } else {
      throw compile_error(next.location,
                          "unexpected token, expected ',' or ')' but was '" +
                              token_to_string(next) + "'");
    }
  }
}

node_idx parse_function_call(Parser *p, node_idx left,
                             LocationRef left_location) {
  p->adv(); // past '('
  Node myself = Node();
  myself.start = left_location;
  myself.tag = NodeTag::FN_CALL;
  node_idx myself_idx = p->nodes.add_dangling(myself);

  p->nodes.add_child(myself_idx, left);

  if (p->peek().tag == TokenTag::BRCLOSE) {
    p->adv();
    return myself_idx;
  }

  parse_add_function_args(p, myself_idx);

  return myself_idx;
}

node_idx parse_left_apply_function(Parser *p, node_idx left) {
  Token identifier = p->peek();
  assert_identifier(identifier);
  p->adv();

  Node myself = Node();
  myself.start = identifier.location;
  myself.tag = NodeTag::FN_CALL;
  node_idx myself_idx = p->nodes.add_dangling(myself);

  Node fn_name = Node();
  fn_name.start = identifier.location;
  fn_name.tag = NodeTag::VAR_REF;
  fn_name.as.IDENTIFIER = identifier.as.IDENTIFIER;
  node_idx fn_name_idx = p->nodes.add_dangling(fn_name);

  p->nodes.add_child(myself_idx, fn_name_idx); // identifier
  p->nodes.add_child(myself_idx, left);        // first argument

  assert_open_br(p->peek());
  p->adv();
  if (p->peek().tag == TokenTag::BRCLOSE) {
    p->adv();
    return myself_idx;
  }

  parse_add_function_args(p, myself_idx);
  return myself_idx;
}
node_idx parse_right_apply_function(Parser *p, node_idx left) {
  Token identifier = p->peek();
  assert_identifier(identifier);
  p->adv();

  Node myself = Node();
  myself.start = identifier.location;
  myself.tag = NodeTag::FN_CALL;
  node_idx myself_idx = p->nodes.add_dangling(myself);

  Node fn_name = Node();
  fn_name.start = identifier.location;
  fn_name.tag = NodeTag::VAR_REF;
  fn_name.as.IDENTIFIER = identifier.as.IDENTIFIER;
  node_idx fn_name_idx = p->nodes.add_dangling(fn_name);

  p->nodes.add_child(myself_idx, fn_name_idx); // identifier

  assert_open_br(p->peek());
  p->adv();

  if (p->peek().tag == TokenTag::BRCLOSE) {
    p->adv();
    p->nodes.add_child(myself_idx, left); // last argument
    return myself_idx;
  }

  parse_add_function_args(p, myself_idx);
  p->nodes.add_child(myself_idx, left); // last argument
  return myself_idx;
}

node_idx parse_regular_infix_operator(Parser *p, node_idx left,
                                      NodeTag operator_tag) {
  LocationRef start = p->peek().location;
  p->adv();
  Node myself = Node();
  myself.tag = operator_tag;
  myself.start = start;
  node_idx myself_idx = p->nodes.add_dangling(myself);
  p->nodes.add_child(myself_idx, left);
  node_idx right = parse_expression_lazy(p);
  p->nodes.add_child(myself_idx, right);
  return myself_idx;
}

node_idx parse_list(Parser *p) {
  Token bropen = p->peek();
  assert_open_br(bropen);
  p->adv();
  Node list_node = Node();
  list_node.start = bropen.location;
  list_node.tag = NodeTag::INTERNAL_LIST;
  node_idx list_idx = p->nodes.add_dangling(list_node);

  if (p->peek().tag == TokenTag::BRCLOSE) {
    p->adv();
    return list_idx;
  }
  while (true) {
    if (p->peek().tag == TokenTag::COMMA) {
      p->adv();
    } else if (p->peek().tag == TokenTag::BRCLOSE) {
      p->adv();
      return list_idx;
    }
    node_idx next_child = parse_expression_eager(p);
    p->nodes.add_child(list_idx, next_child);
  }
}

node_idx parse_consecutive_expression(Parser *p, node_idx left,
                                      NodeTag left_tag,
                                      LocationRef left_location) {
  if (p->eof()) {
    return left;
  }
  Token t = p->peek();
  switch (t.tag) {
  case TokenTag::BROPEN: {
    if (left_tag == NodeTag::VAR_REF) {
      return parse_function_call(p, left, left_location);
    } else {
      return left;
    }
  }
  case TokenTag::DOT: {
    p->adv();
    node_idx myself_idx = parse_left_apply_function(p, left);
    Node myself = p->nodes.at(myself_idx);
    return parse_consecutive_expression(p, myself_idx, myself.tag,
                                        myself.start);
  }
  case TokenTag::BAR: {
    p->adv();
    node_idx myself_idx = parse_right_apply_function(p, left);
    Node myself = p->nodes.at(myself_idx);
    return parse_consecutive_expression(p, myself_idx, myself.tag,
                                        myself.start);
  }
  case TokenTag::OP_ADD: {
    node_idx myself_idx =
        parse_regular_infix_operator(p, left, NodeTag::INFIX_ADD);
    Node myself = p->nodes.at(myself_idx);
    return parse_consecutive_expression(p, myself_idx, myself.tag,
                                        myself.start);
  }
  case TokenTag::OP_SUB: {
    node_idx myself_idx =
        parse_regular_infix_operator(p, left, NodeTag::INFIX_SUB);
    Node myself = p->nodes.at(myself_idx);
    return parse_consecutive_expression(p, myself_idx, myself.tag,
                                        myself.start);
  }
  case TokenTag::OP_MUL: {
    node_idx myself_idx =
        parse_regular_infix_operator(p, left, NodeTag::INFIX_MUL);
    Node myself = p->nodes.at(myself_idx);
    return parse_consecutive_expression(p, myself_idx, myself.tag,
                                        myself.start);
  }
  case TokenTag::OP_DIV: {
    node_idx myself_idx =
        parse_regular_infix_operator(p, left, NodeTag::INFIX_DIV);
    Node myself = p->nodes.at(myself_idx);
    return parse_consecutive_expression(p, myself_idx, myself.tag,
                                        myself.start);
  }
  case TokenTag::OP_MOD: {
    node_idx myself_idx =
        parse_regular_infix_operator(p, left, NodeTag::INFIX_MOD);
    Node myself = p->nodes.at(myself_idx);
    return parse_consecutive_expression(p, myself_idx, myself.tag,
                                        myself.start);
  }
  case TokenTag::OP_EQ: {
    node_idx myself_idx =
        parse_regular_infix_operator(p, left, NodeTag::INFIX_EQ);
    Node myself = p->nodes.at(myself_idx);
    return parse_consecutive_expression(p, myself_idx, myself.tag,
                                        myself.start);
  }
  case TokenTag::OP_NEQ: {
    node_idx myself_idx =
        parse_regular_infix_operator(p, left, NodeTag::INFIX_NEQ);
    Node myself = p->nodes.at(myself_idx);
    return parse_consecutive_expression(p, myself_idx, myself.tag,
                                        myself.start);
  }
  case TokenTag::OP_OR: {
    node_idx myself_idx =
        parse_regular_infix_operator(p, left, NodeTag::INFIX_OR);
    Node myself = p->nodes.at(myself_idx);
    return parse_consecutive_expression(p, myself_idx, myself.tag,
                                        myself.start);
  }
  case TokenTag::OP_AND: {
    node_idx myself_idx =
        parse_regular_infix_operator(p, left, NodeTag::INFIX_AND);
    Node myself = p->nodes.at(myself_idx);
    return parse_consecutive_expression(p, myself_idx, myself.tag,
                                        myself.start);
  }
  case TokenTag::OPERATOR_LT: {
    node_idx myself_idx =
        parse_regular_infix_operator(p, left, NodeTag::INFIX_LT);
    Node myself = p->nodes.at(myself_idx);
    return parse_consecutive_expression(p, myself_idx, myself.tag,
                                        myself.start);
  }
  case TokenTag::OPERATOR_LTE: {
    node_idx myself_idx =
        parse_regular_infix_operator(p, left, NodeTag::INFIX_LTE);
    Node myself = p->nodes.at(myself_idx);
    return parse_consecutive_expression(p, myself_idx, myself.tag,
                                        myself.start);
  }
  case TokenTag::OPERATOR_GT: {
    node_idx myself_idx =
        parse_regular_infix_operator(p, left, NodeTag::INFIX_GT);
    Node myself = p->nodes.at(myself_idx);
    return parse_consecutive_expression(p, myself_idx, myself.tag,
                                        myself.start);
  }
  case TokenTag::OPERATOR_GTE: {
    node_idx myself_idx =
        parse_regular_infix_operator(p, left, NodeTag::INFIX_GTE);
    Node myself = p->nodes.at(myself_idx);
    return parse_consecutive_expression(p, myself_idx, myself.tag,
                                        myself.start);
  }
  case TokenTag::OPERATOR_STRCONCAT: {
    node_idx myself_idx =
        parse_regular_infix_operator(p, left, NodeTag::INFIX_STR_CONCAT);
    Node myself = p->nodes.at(myself_idx);
    return parse_consecutive_expression(p, myself_idx, myself.tag,
                                        myself.start);
  }
  case TokenTag::OP_NOT:
  case TokenTag::BRCLOSE:
  case TokenTag::BRACKETOPEN:
  case TokenTag::BRACKETCLOSE:
  case TokenTag::CURLOPEN:
  case TokenTag::CURLCLOSE:
  case TokenTag::COMMA:
  case TokenTag::LET:
  case TokenTag::SET:
  case TokenTag::IF:
  case TokenTag::ELIF:
  case TokenTag::ELSE:
  case TokenTag::FUNCTION:
  case TokenTag::TRY:
  case TokenTag::EXPECT:
  case TokenTag::ASSIGN:
  case TokenTag::INT:
  case TokenTag::FLOAT:
  case TokenTag::SYMBOL:
  case TokenTag::STRING:
  case TokenTag::NONE:
  case TokenTag::IDENTIFIER:
  case TokenTag::RETURN:
  case TokenTag::FOR:
  case TokenTag::IN:
    return left;
  }
}

node_idx parse_expression_lazy(Parser *p) {
  Token t = p->peek();
  switch (t.tag) {
  case TokenTag::INT: {
    Node node = make_node(t.location, NodeTag::LITERAL_INT, t.as.INT);
    p->adv();
    return p->nodes.add_dangling(node);
  }
  case TokenTag::FLOAT: {
    Node node = make_node(t.location, NodeTag::LITERAL_FLOAT, t.as.FLOAT);
    p->adv();
    return p->nodes.add_dangling(node);
  }
  case TokenTag::SYMBOL: {
    Node node = make_node(t.location, NodeTag::LITERAL_SYMBOL, t.as.SYMBOL);
    p->adv();
    return p->nodes.add_dangling(node);
  }
  case TokenTag::STRING: {
    Node node = make_node(t.location, NodeTag::LITERAL_STRING, t.as.STR);
    p->adv();
    return p->nodes.add_dangling(node);
  }
  case TokenTag::NONE: {
    Node node = make_node(t.location, NodeTag::LITERAL_NONE);
    p->adv();
    return p->nodes.add_dangling(node);
  }
  case TokenTag::IDENTIFIER: {
    Node node =
        make_node_identifier(t.location, NodeTag::VAR_REF, t.as.IDENTIFIER);
    p->adv();
    node_idx var_ref = p->nodes.add_dangling(node);
    if (!p->eof() && p->peek().tag == TokenTag::BROPEN) {
      return parse_function_call(p, var_ref, node.start);
    } else {
      return var_ref;
    }
  }
  case TokenTag::LET:
    throw compile_error(t.location,
                        "unexpected token 'let' is not an expression");
  case TokenTag::SET:
    throw compile_error(t.location,
                        "unexpected token 'set' is not an expression");
  case TokenTag::IF:
    throw compile_error(t.location, "unexpected token 'if'");
  case TokenTag::ELIF:
    throw compile_error(t.location, "unexpected token 'elif'");
  case TokenTag::ELSE:
    throw compile_error(t.location, "unexpected token 'else'");
  case TokenTag::FUNCTION:
    throw compile_error(t.location, "unexpected token 'function'");
  case TokenTag::TRY:
    return parse_try_lazy(p);
  case TokenTag::EXPECT:
    return parse_expect_lazy(p);
  case TokenTag::ASSIGN:
    throw compile_error(t.location, "unexpected token '='");
  case TokenTag::BROPEN:
    throw compile_error(t.location, "unexpected token '('");
  case TokenTag::BRCLOSE:
    throw compile_error(t.location, "unexpected token ')'");
  case TokenTag::BRACKETOPEN: {
    p->adv();
    node_idx expr_idx = parse_expression_eager(p);
    Token close = p->peek();
    assert_close_bracket(close);
    p->adv();
    return expr_idx;
  }
  case TokenTag::BRACKETCLOSE:
    throw compile_error(t.location, "unexpected token ']'");
  case TokenTag::CURLOPEN:
    throw compile_error(t.location, "unexpected token '{'");
  case TokenTag::CURLCLOSE:
    throw compile_error(t.location, "unexpected token '}'");
  case TokenTag::COMMA:
    throw compile_error(t.location, "unexpected token ','");
  case TokenTag::DOT:
    throw compile_error(t.location, "unexpected token '.'");
  case TokenTag::BAR:
    throw compile_error(t.location, "unexpected token '|'");
  case TokenTag::OP_ADD:
    throw compile_error(t.location, "unexpected token '+'");
  case TokenTag::OP_SUB: {
    LocationRef start = p->peek().location;
    p->adv();
    Node inv_node;
    inv_node.start = start;
    inv_node.tag = NodeTag::PREFIX_INVERS;
    node_idx myself_idx = p->nodes.add_dangling(inv_node);
    node_idx value_idx = parse_expression_lazy(p);
    p->nodes.add_child(myself_idx, value_idx);
    return myself_idx;
  }
  case TokenTag::OP_MUL:
    throw compile_error(t.location, "unexpected token '*'");
  case TokenTag::OP_DIV:
    throw compile_error(t.location, "unexpected token '/'");
  case TokenTag::OP_MOD:
    throw compile_error(t.location, "unexpected token 'mod'");
  case TokenTag::OP_EQ:
    throw compile_error(t.location, "unexpected token '=='");
  case TokenTag::OP_NEQ:
    throw compile_error(t.location, "unexpected token '!='");
  case TokenTag::OP_NOT: {
    LocationRef start = p->peek().location;
    p->adv();
    Node not_node;
    not_node.start = start;
    not_node.tag = NodeTag::PREFIX_NOT;
    node_idx myself_idx = p->nodes.add_dangling(not_node);
    node_idx value_idx = parse_expression_lazy(p);
    p->nodes.add_child(myself_idx, value_idx);
    return myself_idx;
  }
  case TokenTag::OP_OR:
    throw compile_error(t.location, "unexpected token 'or'");
  case TokenTag::OP_AND:
    throw compile_error(t.location, "unexpected token 'and'");
  case TokenTag::OPERATOR_LT:
    throw compile_error(t.location, "unexpected token '<'");
  case TokenTag::OPERATOR_LTE:
    throw compile_error(t.location, "unexpected token '<='");
  case TokenTag::OPERATOR_GT:
    throw compile_error(t.location, "unexpected token '>'");
  case TokenTag::OPERATOR_GTE:
    throw compile_error(t.location, "unexpected token '>='");
  case TokenTag::OPERATOR_STRCONCAT:
    throw compile_error(t.location, "unexpected token '<>'");
  case TokenTag::RETURN:
    throw compile_error(t.location, "unexpected token 'return'");
  case TokenTag::FOR:
    throw compile_error(t.location, "unexpected token 'for'");
  case TokenTag::IN:
    throw compile_error(t.location, "unexpected token 'in'");
  }
}

node_idx parse_let(Parser *p) {
  LocationRef start = p->peek().location;
  p->adv();
  Token identifier = p->peek();
  assert_identifier(identifier);
  p->adv();
  assert_assign(p->peek());
  p->adv(); // move to expression body

  // add myself
  Node var_def = Node();
  var_def.as.IDENTIFIER = identifier.as.IDENTIFIER;
  var_def.start = start;
  var_def.tag = NodeTag::VAR_DEF;
  node_idx myself = p->nodes.add_dangling(var_def);
  // hook up child
  node_idx first_child = parse_expression_eager(p);
  p->nodes.add_child(myself, first_child);
  return myself;
}
node_idx parse_set(Parser *p) {
  LocationRef start = p->peek().location;
  p->adv();
  Token identifier = p->peek();
  assert_identifier(identifier);
  p->adv();
  assert_assign(p->peek());
  p->adv(); // move to expression body

  // add myself
  Node var_def = Node();
  var_def.as.IDENTIFIER = identifier.as.IDENTIFIER;
  var_def.start = start;
  var_def.tag = NodeTag::VAR_SET;
  node_idx myself = p->nodes.add_dangling(var_def);
  // hook up child
  node_idx first_child = parse_expression_eager(p);
  p->nodes.add_child(myself, first_child);
  return myself;
}

node_idx parse_block(Parser *p) {
  Token curbropen = p->peek();
  assert_open_cur(curbropen);
  p->adv();
  if (p->peek().tag == TokenTag::CURLCLOSE) {
    p->adv();
    return node_idx{0};
  }
  node_idx dangling_first_elem_idx = parse_one(p);
  while (true) {
    if (p->peek().tag == TokenTag::CURLCLOSE) {
      p->adv();
      return dangling_first_elem_idx;
    }
    node_idx next_child = parse_one(p);
    p->nodes.add_next_child(dangling_first_elem_idx, next_child);
  }
}

node_idx parse_if(Parser *p) {
  Node myself = Node();
  myself.start = p->peek().location;
  myself.tag = NodeTag::IF;
  node_idx myself_idx = p->nodes.add_dangling(myself);

  // parsing 'if' condition
  {
    Node if_cond = Node();
    if_cond.start = myself.start;
    if_cond.tag = NodeTag::INTERNAL_PARTIAL_CONDITION;
    node_idx if_cond_idx = p->nodes.add_dangling(if_cond);
    p->nodes.add_child(myself_idx, if_cond_idx);
    p->adv();

    Token bropen = p->peek();
    assert_open_br(bropen);
    p->adv();
    node_idx condition_idx = parse_expression_eager(p);
    p->nodes.add_child(if_cond_idx, condition_idx);
    Token brclose = p->peek();
    assert_close_br(brclose);
    p->adv();
    node_idx body_idx = parse_block(p);
    if (!body_idx.is_null()) {
      p->nodes.add_child(if_cond_idx, body_idx);
    }
  }

  // parsing 'elif' conditions
  while (!p->eof() && p->peek().tag == TokenTag::ELIF) {
    Node elif_cond = Node();
    elif_cond.start = p->peek().location;
    elif_cond.tag = NodeTag::INTERNAL_PARTIAL_CONDITION;
    node_idx elif_cond_idx = p->nodes.add_dangling(elif_cond);
    p->nodes.add_child(myself_idx, elif_cond_idx);
    p->adv();

    Token bropen = p->peek();
    assert_open_br(bropen);
    p->adv();
    node_idx condition_idx = parse_expression_eager(p);
    p->nodes.add_child(elif_cond_idx, condition_idx);
    Token brclose = p->peek();
    assert_close_br(brclose);
    p->adv();
    node_idx body_idx = parse_block(p);
    if (!body_idx.is_null()) {
      p->nodes.add_child(elif_cond_idx, body_idx);
    }
  }

  // parsing 'else' condition
  if (!p->eof() && p->peek().tag == TokenTag::ELSE) {
    Node else_cond = Node();
    else_cond.start = p->peek().location;
    else_cond.tag = NodeTag::INTERNAL_PARTIAL_DEFAULT_CONDITION;
    node_idx else_cond_idx = p->nodes.add_dangling(else_cond);
    p->nodes.add_child(myself_idx, else_cond_idx);
    p->adv();

    node_idx body_idx = parse_block(p);
    if (!body_idx.is_null()) {
      p->nodes.add_child(else_cond_idx, body_idx);
    }
  }

  return myself_idx;
}

node_idx parse_function(Parser *p) {
  LocationRef start = p->peek().location;
  p->adv();
  Token identifier = p->peek();
  assert_identifier(identifier);
  p->adv();
  Node myself = Node();
  myself.start = start;
  myself.as.IDENTIFIER = identifier.as.IDENTIFIER;
  myself.tag = NodeTag::FN_DEF;
  node_idx myself_idx = p->nodes.add_dangling(myself);

  // arguments
  node_idx arg_list_idx = parse_list(p);
  p->nodes.add_child(myself_idx, arg_list_idx);
  node_idx body_idx = parse_block(p);
  if (!body_idx.is_null()) {
    p->nodes.add_child(myself_idx, body_idx);
  }
  return myself_idx;
}

node_idx parse_return(Parser *p) {
  LocationRef start = p->peek().location;
  p->adv();
  Node myself = Node();
  myself.start = start;
  myself.tag = NodeTag::RETURN;
  node_idx myself_idx = p->nodes.add_dangling(myself);

  // argument
  node_idx value_idx = parse_expression_eager(p);
  p->nodes.add_child(myself_idx, value_idx);
  return myself_idx;
}
node_idx parse_for(Parser *p) {
  LocationRef start = p->peek().location;
  p->adv(); // move to loop param

  Token identifier = p->peek();
  assert_identifier(identifier);

  Node myself = Node();
  myself.start = start;
  myself.tag = NodeTag::FOR_LOOP;
  myself.as.IDENTIFIER = identifier.as.IDENTIFIER;
  node_idx myself_idx = p->nodes.add_dangling(myself);

  p->adv(); // move to 'in'
  assert_in(p->peek());
  p->adv(); // move to list
  node_idx list_idx = parse_expression_eager(p);
  p->nodes.add_child(myself_idx, list_idx);

  node_idx body_idx = parse_block(p);
  if (!body_idx.is_null()) {
    p->nodes.add_child(myself_idx, body_idx);
  }
  return myself_idx;
}

node_idx parse_one(Parser *p) {
  Token token = p->peek();
  switch (token.tag) {
  case TokenTag::INT:
    return parse_expression_eager(p);
  case TokenTag::FLOAT:
    return parse_expression_eager(p);
  case TokenTag::SYMBOL:
    return parse_expression_eager(p);
  case TokenTag::STRING:
    return parse_expression_eager(p);
  case TokenTag::NONE:
    return parse_expression_eager(p);
  case TokenTag::LET:
    return parse_let(p);
  case TokenTag::SET:
    return parse_set(p);
  case TokenTag::IF:
    return parse_if(p);
  case TokenTag::ELIF:
    throw compile_error(token.location, "unexpected token");
  case TokenTag::ELSE:
    throw compile_error(token.location, "unexpected token");
  case TokenTag::FUNCTION:
    return parse_function(p);
  case TokenTag::TRY:
    return parse_expression_eager(p);
  case TokenTag::EXPECT:
    return parse_expression_eager(p);
  case TokenTag::RETURN:
    return parse_return(p);
  case TokenTag::FOR:
    return parse_for(p);
  case TokenTag::ASSIGN:
    throw compile_error(token.location, "unexpected token");
  case TokenTag::BROPEN:
    throw compile_error(token.location, "unexpected token");
  case TokenTag::BRCLOSE:
    throw compile_error(token.location, "unexpected token");
  case TokenTag::BRACKETOPEN:
    return parse_expression_eager(p);
  case TokenTag::BRACKETCLOSE:
    throw compile_error(token.location, "unexpected token");
  case TokenTag::CURLOPEN:
    throw compile_error(token.location, "unexpected token");
  case TokenTag::CURLCLOSE:
    throw compile_error(token.location, "unexpected token");
  case TokenTag::COMMA:
    throw compile_error(token.location, "unexpected token");
  case TokenTag::DOT:
    throw compile_error(token.location, "unexpected token");
  case TokenTag::BAR:
    throw compile_error(token.location, "unexpected token");
  case TokenTag::OP_ADD:
    throw compile_error(token.location, "unexpected token");
  case TokenTag::OP_SUB:
    throw compile_error(token.location, "unexpected token");
  case TokenTag::OP_MUL:
    throw compile_error(token.location, "unexpected token");
  case TokenTag::OP_DIV:
    throw compile_error(token.location, "unexpected token");
  case TokenTag::OP_MOD:
    throw compile_error(token.location, "unexpected token");
  case TokenTag::OP_EQ:
    throw compile_error(token.location, "unexpected token");
  case TokenTag::OP_NEQ:
    throw compile_error(token.location, "unexpected token");
  case TokenTag::OP_NOT:
    return parse_expression_eager(p);
  case TokenTag::OP_OR:
    throw compile_error(token.location, "unexpected token");
  case TokenTag::OP_AND:
    throw compile_error(token.location, "unexpected token");
  case TokenTag::OPERATOR_LT:
    throw compile_error(token.location, "unexpected token");
  case TokenTag::OPERATOR_LTE:
    throw compile_error(token.location, "unexpected token");
  case TokenTag::OPERATOR_GT:
    throw compile_error(token.location, "unexpected token");
  case TokenTag::OPERATOR_GTE:
    throw compile_error(token.location, "unexpected token");
  case TokenTag::OPERATOR_STRCONCAT:
    throw compile_error(token.location, "unexpected token");
  case TokenTag::IDENTIFIER:
    return parse_expression_eager(p);
  case TokenTag::IN:
    throw compile_error(token.location, "unexpected token");
  }
}
}; // namespace

nodes parse(const std::vector<Token> &program) {
  if (program.size() == 0) {
    return {};
  }

  // initialization of parser and nodes object
  Parser p;
  p.tokens = program;
  p.pos = 0;
  p.len = p.tokens.size();
  p.nodes = nodes();
  // parsing
  while (!p.eof()) {
    p.nodes.add_sibling(parse_one(&p));
  }
  return p.nodes;
}
