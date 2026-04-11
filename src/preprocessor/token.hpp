#pragma once

#include "../interned_string.hpp"
#include "../location.hpp"
#include "../symbol.hpp"
#include <cstdint>

enum class TokenTag : uint8_t {
  INT,
  FLOAT,
  SYMBOL,
  STRING,
  NONE,
  LET,
  SET,
  IF,
  ELIF,
  ELSE,
  FUNCTION,
  TRY,
  EXPECT,
  REF,
  RETURN,
  CONTINUE,
  BREAK,
  FOR,
  IN,
  ASSIGN,
  BROPEN,
  BRCLOSE,
  BRACKETOPEN,
  BRACKETCLOSE,
  CURLOPEN,
  CURLCLOSE,
  COMMA,
  DOT,
  BAR,
  OP_ADD,
  OP_SUB,
  OP_MUL,
  OP_DIV,
  OP_MOD,
  OP_EQ,
  OP_NEQ,
  OP_NOT,
  OP_OR,
  OP_AND,
  OPERATOR_LT,
  OPERATOR_LTE,
  OPERATOR_GT,
  OPERATOR_GTE,
  OPERATOR_STRCONCAT,
  IDENTIFIER,
};

struct Token {
  LocationRef location;
  union {
    int64_t INT;
    double FLOAT;
    InternedString STR;
    Symbol SYMBOL;
    InternedString IDENTIFIER;
  } as;
  TokenTag tag;
};

inline std::string token_to_string(const Token &token) {
  switch (token.tag) {
  case TokenTag::INT:
    return std::to_string(token.as.INT);
  case TokenTag::FLOAT:
    return std::to_string(token.as.FLOAT);
  case TokenTag::SYMBOL:
    return resolve_symbol(token.as.SYMBOL);
  case TokenTag::STRING:
    return "\"" + resolve_interned_string(token.as.STR) + "\"";
  case TokenTag::NONE:
    return "none";
  case TokenTag::LET:
    return "let";
  case TokenTag::SET:
    return "set";
  case TokenTag::IF:
    return "if";
  case TokenTag::ELIF:
    return "elif";
  case TokenTag::ELSE:
    return "else";
  case TokenTag::FUNCTION:
    return "function";
  case TokenTag::TRY:
    return "try";
  case TokenTag::EXPECT:
    return "expect";
  case TokenTag::ASSIGN:
    return "=";
  case TokenTag::BROPEN:
    return "(";
  case TokenTag::BRCLOSE:
    return ")";
  case TokenTag::CURLOPEN:
    return "{";
  case TokenTag::CURLCLOSE:
    return "}";
  case TokenTag::COMMA:
    return ",";
  case TokenTag::DOT:
    return ".";
  case TokenTag::BAR:
    return "|";
  case TokenTag::OP_ADD:
    return "+";
  case TokenTag::OP_SUB:
    return "-";
  case TokenTag::OP_MUL:
    return "*";
  case TokenTag::OP_DIV:
    return "/";
  case TokenTag::OP_MOD:
    return "mod";
  case TokenTag::OP_EQ:
    return "==";
  case TokenTag::OP_NEQ:
    return "!=";
  case TokenTag::OP_NOT:
    return "not";
  case TokenTag::OP_OR:
    return "or";
  case TokenTag::OP_AND:
    return "and";
  case TokenTag::OPERATOR_LT:
    return "<";
  case TokenTag::OPERATOR_LTE:
    return "<=";
  case TokenTag::OPERATOR_GT:
    return ">";
  case TokenTag::OPERATOR_GTE:
    return ">=";
  case TokenTag::OPERATOR_STRCONCAT:
    return "<>";
  case TokenTag::IDENTIFIER:
    return resolve_interned_string(token.as.IDENTIFIER);
  case TokenTag::RETURN:
    return "return";
  case TokenTag::BRACKETOPEN:
    return "[";
  case TokenTag::BRACKETCLOSE:
    return "]";
  case TokenTag::FOR:
    return "for";
  case TokenTag::IN:
    return "in";
  case TokenTag::CONTINUE:
    return "continue";
  case TokenTag::BREAK:
    return "break";
  case TokenTag::REF:
    return "ref";
  }
}
