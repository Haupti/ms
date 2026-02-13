#include "../../lib/asap/util.hpp"
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
      throw runtime_error("unexpected EOF\n");
    }
    return tokens.at(pos);
  }
};

void assert_identifier(Token t) {
  if (t.tag != TokenTag::IDENTIFIER) {
    throw runtime_error("expected an identifier\n");
  }
}
void assert_assign(Token t) {
  if (t.tag != TokenTag::ASSIGN) {
    throw runtime_error("expected '='\n");
  }
}
void assert_close_br(Token t) {
  if (t.tag != TokenTag::BRCLOSE) {
    throw runtime_error("expected ')'\n");
  }
}

node_idx parse_consecutive_expression(Parser *p, node_idx left,
                                      NodeTag left_tag,
                                      LocationRef left_location);
node_idx parse_expression_lazy(Parser *p);
node_idx parse_expression_eager(Parser *p) {
  node_idx left = parse_expression_lazy(p);
  Node left_node = p->nodes.at(left);
  return parse_consecutive_expression(p, left, left_node.tag, left_node.start);
}

node_idx parse_function_call(Parser *p, node_idx left,
                             LocationRef left_location) {
  p->adv(); // past '('
  Node fn_call_node = Node();
  fn_call_node.start = left_location;
  fn_call_node.tag = NodeTag::FN_CALL;
  node_idx myself_idx = p->nodes.add_dangling(fn_call_node);
  p->nodes.add_child(myself_idx, left);
  if (p->peek().tag == TokenTag::BRCLOSE) {
    return myself_idx;
  }
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
      throw runtime_error("unexpected token, expected ',' or ')'");
    }
  }
  return myself_idx;
}

node_idx parse_add_operator(Parser *p, node_idx left) {
  LocationRef start = p->peek().location;
  p->adv();
  Node myself = Node();
  myself.tag = NodeTag::INFIX_ADD;
  myself.start = start;
  node_idx myself_idx = p->nodes.add_dangling(myself);
  p->nodes.add_child(myself_idx, left);
  node_idx right = parse_expression_lazy(p);
  p->nodes.add_child(myself_idx, right);
  return myself_idx;
}

// TODO expression parsing with infix stuff
// 1. parse lazy
// 2. pass result to eager parsing step
// 3. that step checks if the next token is an infix operator
// 4a. if so -> parse the next expression lazy , build the final node and pass
// that to the eager parsing 4b. if not -> just return the previous lazy result
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
  case TokenTag::DOT:
    throw runtime_error("NOT YET IMPLEMENTED");
  case TokenTag::BAR:
    throw runtime_error("NOT YET IMPLEMENTED");
  case TokenTag::OP_ADD:
    return parse_add_operator(p, left);
  case TokenTag::OP_SUB:
    throw runtime_error("NOT YET IMPLEMENTED");
  case TokenTag::OP_MUL:
    throw runtime_error("NOT YET IMPLEMENTED");
  case TokenTag::OP_DIV:
    throw runtime_error("NOT YET IMPLEMENTED");
  case TokenTag::OP_MOD:
    throw runtime_error("NOT YET IMPLEMENTED");
  case TokenTag::OP_EQ:
    throw runtime_error("NOT YET IMPLEMENTED");
  case TokenTag::OP_NEQ:
    throw runtime_error("NOT YET IMPLEMENTED");
  case TokenTag::OP_NOT:
    throw runtime_error("NOT YET IMPLEMENTED");
  case TokenTag::OP_OR:
    throw runtime_error("NOT YET IMPLEMENTED");
  case TokenTag::OP_AND:
    throw runtime_error("NOT YET IMPLEMENTED");
  case TokenTag::OPERATOR_LT:
    throw runtime_error("NOT YET IMPLEMENTED");
  case TokenTag::OPERATOR_LTE:
    throw runtime_error("NOT YET IMPLEMENTED");
  case TokenTag::OPERATOR_GT:
    throw runtime_error("NOT YET IMPLEMENTED");
  case TokenTag::OPERATOR_GTE:
    throw runtime_error("NOT YET IMPLEMENTED");
  case TokenTag::OPERATOR_STRCONCAT:
    throw runtime_error("NOT YET IMPLEMENTED");
  case TokenTag::INT:
  case TokenTag::FLOAT:
  case TokenTag::SYMBOL:
  case TokenTag::STRING:
  case TokenTag::BRCLOSE:
  case TokenTag::CURLOPEN:
  case TokenTag::CURLCLOSE:
  case TokenTag::COMMA:
  case TokenTag::LET:
  case TokenTag::SET:
  case TokenTag::AT:
  case TokenTag::IF:
  case TokenTag::ELIF:
  case TokenTag::ELSE:
  case TokenTag::FUNCTION:
  case TokenTag::TRY:
  case TokenTag::EXPECT:
  case TokenTag::ASSIGN:
  case TokenTag::IDENTIFIER:
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
  case TokenTag::IDENTIFIER: {
    Node node =
        make_node_identifier(t.location, NodeTag::VAR_REF, t.as.IDENTIFIER);
    p->adv();
    return p->nodes.add_dangling(node);
  }
  case TokenTag::LET:
    throw compile_error(t.location,
                        "unexpected token 'let' is not an expression");
  case TokenTag::SET:
    throw compile_error(t.location,
                        "unexpected token 'set' is not an expression");
  case TokenTag::AT:
    throw compile_error(t.location, "unexpected token 'at'");
  case TokenTag::IF:
    throw compile_error(t.location, "unexpected token 'if'");
  case TokenTag::ELIF:
    throw compile_error(t.location, "unexpected token 'elif'");
  case TokenTag::ELSE:
    throw compile_error(t.location, "unexpected token 'else'");
  case TokenTag::FUNCTION:
    throw compile_error(t.location, "unexpected token 'function'");
  case TokenTag::TRY:
    throw runtime_error("NOT YET IMPLEMENTED");
  case TokenTag::EXPECT:
    throw runtime_error("NOT YET IMPLEMENTED");
  case TokenTag::ASSIGN:
    throw compile_error(t.location, "unexpected token '='");
  case TokenTag::BROPEN: {
    p->adv();
    node_idx expr_idx = parse_expression_eager(p);
    Token close = p->peek();
    assert_close_br(close);
    p->adv();
    return expr_idx;
  }
  case TokenTag::BRCLOSE:
    throw compile_error(t.location, "unexpected token ')'");
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
    break;
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
  node_idx first_child = parse_expression_lazy(p);
  p->nodes.add_child(myself, first_child);
  return myself;
}
}; // namespace

nodes parse(const std::vector<Token> program) {
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
  Token token;
  while (!p.eof()) {
    token = p.peek();
    switch (token.tag) {
    case TokenTag::INT: {
      p.nodes.add_sibling(parse_expression_eager(&p));
    } break;
    case TokenTag::FLOAT: {
      p.nodes.add_sibling(parse_expression_eager(&p));
    } break;
    case TokenTag::SYMBOL: {
      p.nodes.add_sibling(parse_expression_eager(&p));
    } break;
    case TokenTag::STRING: {
      p.nodes.add_sibling(parse_expression_eager(&p));
    } break;
    case TokenTag::LET:
      p.nodes.add_sibling(parse_let(&p));
      break;
    case TokenTag::SET:
      throw runtime_error("NOT YET IMPLEMENTED");
    case TokenTag::AT:
      throw compile_error(token.location, "unexpected token");
    case TokenTag::IF:
      throw runtime_error("NOT YET IMPLEMENTED");
    case TokenTag::ELIF:
      throw compile_error(token.location, "unexpected token");
    case TokenTag::ELSE:
      throw compile_error(token.location, "unexpected token");
    case TokenTag::FUNCTION:
      throw runtime_error("NOT YET IMPLEMENTED");
    case TokenTag::TRY:
      throw runtime_error("NOT YET IMPLEMENTED");
    case TokenTag::EXPECT:
      throw runtime_error("NOT YET IMPLEMENTED");
    case TokenTag::ASSIGN:
      throw compile_error(token.location, "unexpected token");
    case TokenTag::BROPEN:
      p.nodes.add_sibling(parse_expression_eager(&p));
      break;
    case TokenTag::BRCLOSE:
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
      p.nodes.add_sibling(parse_expression_lazy(&p));
      break;
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
    case TokenTag::IDENTIFIER: {
      p.nodes.add_sibling(make_node_identifier(
          token.location, NodeTag::LITERAL_STRING, token.as.IDENTIFIER));
      p.adv();
    } break;
    }
  }
  return p.nodes;
}
