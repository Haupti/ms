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

node_idx parse_expression(Parser *p) {
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
  case TokenTag::SET:
  case TokenTag::AT:
  case TokenTag::IF:
  case TokenTag::ELIF:
  case TokenTag::ELSE:
  case TokenTag::FUNCTION:
  case TokenTag::TRY:
  case TokenTag::EXPECT:
  case TokenTag::ASSIGN:
  case TokenTag::BROPEN:
  case TokenTag::BRCLOSE:
  case TokenTag::CURLOPEN:
  case TokenTag::CURLCLOSE:
  case TokenTag::COMMA:
  case TokenTag::DOT:
  case TokenTag::BAR:
  case TokenTag::OP_ADD:
  case TokenTag::OP_SUB:
  case TokenTag::OP_MUL:
  case TokenTag::OP_DIV:
  case TokenTag::OP_MOD:
  case TokenTag::OP_EQ:
  case TokenTag::OP_NEQ:
  case TokenTag::OP_NOT:
  case TokenTag::OP_OR:
  case TokenTag::OP_AND:
  case TokenTag::OPERATOR_LT:
  case TokenTag::OPERATOR_LTE:
  case TokenTag::OPERATOR_GT:
  case TokenTag::OPERATOR_GTE:
  case TokenTag::OPERATOR_STRCONCAT:
    throw runtime_error("NOT YET IMPLEMENTED");
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
  node_idx first_child = parse_expression(p);
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
      p.nodes.add_sibling(
          make_node(token.location, NodeTag::LITERAL_INT, token.as.INT));
      p.adv();
    } break;
    case TokenTag::FLOAT: {
      p.nodes.add_sibling(
          make_node(token.location, NodeTag::LITERAL_FLOAT, token.as.FLOAT));
      p.adv();
    } break;
    case TokenTag::SYMBOL: {
      p.nodes.add_sibling(
          make_node(token.location, NodeTag::LITERAL_SYMBOL, token.as.SYMBOL));
      p.adv();
    } break;
    case TokenTag::STRING: {
      p.nodes.add_sibling(
          make_node(token.location, NodeTag::LITERAL_STRING, token.as.STR));
      p.adv();
    } break;
    case TokenTag::LET:
      p.nodes.add_sibling(parse_let(&p));
      break;
    case TokenTag::SET:
      throw runtime_error("NOT YET IMPLEMENTED");
    case TokenTag::AT:
      throw runtime_error("unexpected token");
    case TokenTag::IF:
      throw runtime_error("NOT YET IMPLEMENTED");
    case TokenTag::ELIF:
      throw runtime_error("unexpected token");
    case TokenTag::ELSE:
      throw runtime_error("unexpected token");
    case TokenTag::FUNCTION:
      throw runtime_error("NOT YET IMPLEMENTED");
    case TokenTag::TRY:
      throw runtime_error("unexpected token");
    case TokenTag::EXPECT:
      throw runtime_error("unexpected token");
    case TokenTag::ASSIGN:
      throw runtime_error("unexpected token");
    case TokenTag::BROPEN:
    case TokenTag::BRCLOSE:
      throw runtime_error("unexpected token");
    case TokenTag::CURLOPEN:
      throw runtime_error("NOT YET IMPLEMENTED");
    case TokenTag::CURLCLOSE:
      throw runtime_error("unexpected token");
    case TokenTag::COMMA:
      throw runtime_error("unexpected token");
    case TokenTag::DOT:
      throw runtime_error("unexpected token");
    case TokenTag::BAR:
      throw runtime_error("unexpected token");
    case TokenTag::OP_ADD:
      throw runtime_error("unexpected token");
    case TokenTag::OP_SUB:
      throw runtime_error("unexpected token");
    case TokenTag::OP_MUL:
      throw runtime_error("unexpected token");
    case TokenTag::OP_DIV:
      throw runtime_error("unexpected token");
    case TokenTag::OP_MOD:
      throw runtime_error("unexpected token");
    case TokenTag::OP_EQ:
      throw runtime_error("unexpected token");
    case TokenTag::OP_NEQ:
      throw runtime_error("unexpected token");
    case TokenTag::OP_NOT:
      throw runtime_error("NOT YET IMPLEMENTED");
    case TokenTag::OP_OR:
      throw runtime_error("unexpected token");
    case TokenTag::OP_AND:
      throw runtime_error("unexpected token");
    case TokenTag::OPERATOR_LT:
      throw runtime_error("unexpected token");
    case TokenTag::OPERATOR_LTE:
      throw runtime_error("unexpected token");
    case TokenTag::OPERATOR_GT:
      throw runtime_error("unexpected token");
    case TokenTag::OPERATOR_GTE:
      throw runtime_error("unexpected token");
    case TokenTag::OPERATOR_STRCONCAT:
      throw runtime_error("unexpected token");
    case TokenTag::IDENTIFIER: {
      p.nodes.add_sibling(make_node_identifier(
          token.location, NodeTag::LITERAL_STRING, token.as.IDENTIFIER));
      p.adv();
    } break;
    }
  }
  return p.nodes;
}
