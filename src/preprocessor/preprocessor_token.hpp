#pragma once
#include "../interned_string.hpp"
#include "../location.hpp"
#include "../symbol.hpp"

#define __PREPROCESSOR_TOKEN_NONE_VAL {.INT = 0}

enum class PpTokenTag : uint8_t {
  INT,
  FLOAT,
  SYMBOL,
  STRING,
  LET,
  SET,
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
  INCLUDE_MACRO,
  DEFINE_MACRO,
  FSET_MACRO,
  FUNSET_MACRO,
  IFFSET_MACRO,
  IFNFSET_MACRO,
  ENDIF_MACRO,
};

struct PreprocessorToken {
  LocationRef location;
  union {
    int64_t INT;
    double FLOAT;
    InternedString STR;
    Symbol SYMBOL;
    InternedString IDENTIFIER;
  } as;
  PpTokenTag tag;
};

inline PreprocessorToken build_pptoken_int(const LocationRef &location,
                                           int64_t value) {
  return PreprocessorToken{
      .location = location,
      .as = {.INT = value},
      .tag = PpTokenTag::INT,
  };
}

inline PreprocessorToken build_pptoken_float(const LocationRef &location,
                                             double value) {
  return PreprocessorToken{
      .location = location,
      .as = {.FLOAT = value},
      .tag = PpTokenTag::FLOAT,
  };
}

inline PreprocessorToken build_pptoken_string(const LocationRef &location,
                                              const InternedString &value) {
  return PreprocessorToken{
      .location = location,
      .as = {.STR = value},
      .tag = PpTokenTag::STRING,
  };
}

inline PreprocessorToken build_pptoken_symbol(const LocationRef &location,
                                              const Symbol &value) {
  return PreprocessorToken{
      .location = location,
      .as = {.SYMBOL = value},
      .tag = PpTokenTag::SYMBOL,
  };
}

inline PreprocessorToken build_pptoken_identifier(const LocationRef &location,
                                                  const InternedString &value) {
  return PreprocessorToken{
      .location = location,
      .as = {.IDENTIFIER = value},
      .tag = PpTokenTag::IDENTIFIER,
  };
}

inline PreprocessorToken build_pptoken(const LocationRef &location,
                                       PpTokenTag tag) {
  return PreprocessorToken{
      .location = location,
      .as = __PREPROCESSOR_TOKEN_NONE_VAL,
      .tag = tag,
  };
}
