#include "../preprocessor/token.hpp"
#include "ast_node.hpp"
#include <vector>

using namespace std;

namespace {
struct Parser {
  vector<Token> tokens;
  nodes nodes;
  uint64_t pos;
  uint64_t len;
  bool eof() { return pos >= len; }
};
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
  p.nodes._nodes.reserve(program.size() / 10);
  ASTNode nil = make_node(LocationRef{0}, ASTNodeTag::NIL);
  p.nodes._nodes.push_back(nil);
  // parsing
  Token token = program.at(0);
  while (!p.eof()) {
    switch (token.tag) {
    case TokenTag::INT: {
      p.nodes.add(
          make_node(token.location, ASTNodeTag::LITERAL_INT, token.as.INT));
    } break;
    case TokenTag::FLOAT: {
      p.nodes.add(
          make_node(token.location, ASTNodeTag::LITERAL_FLOAT, token.as.FLOAT));
    } break;
    case TokenTag::SYMBOL: {
      p.nodes.add(make_node(token.location, ASTNodeTag::LITERAL_SYMBOL,
                            token.as.SYMBOL));
    } break;
    case TokenTag::STRING: {
      p.nodes.add(
          make_node(token.location, ASTNodeTag::LITERAL_STRING, token.as.STR));
    } break;
    case TokenTag::LET:
    case TokenTag::SET:
    case TokenTag::AT:
      throw runtime_error("unexpected token");
    case TokenTag::IF:
    case TokenTag::ELIF:
      throw runtime_error("unexpected token");
    case TokenTag::ELSE:
      throw runtime_error("unexpected token");
    case TokenTag::FUNCTION:
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
      p.nodes.add(make_node_identifier(
          token.location, ASTNodeTag::LITERAL_STRING, token.as.IDENTIFIER));
    } break;
    }
  }
  return p.nodes;
}
