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
  LET,
  SET,
  AT,
  IF,
  ELIF,
  ELSE,
  FUNCTION,
  TRY,
  EXPECT,
  RETURN,
  ASSIGN,
  BROPEN,
  BRCLOSE,
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
