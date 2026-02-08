#pragma once
#include "interned_string.hpp"
#include "location.hpp"
#include "symbol.hpp"

enum class PreprocessorTokenTag : uint8_t {
  INT,
  FLOAT,
  BOOL,
  SYMBOL,
  STRING,
  NONE,
  LET,
  SET,
  IF,
  ELIF,
  ELSE,
  FUNCTION,
  ASSIGN,
  BROPEN,
  BRCLOSE,
  CURLOPEN,
  CURLCLOSE,
  COMMA,
  DOT,
  BAR,
  OPERATOR_ADD,
  OPERATOR_SUB,
  OPERATOR_MUL,
  OPERATOR_DIV,
  OPERATOR_MOD,
  OPERATOR_EQ,
  OPERATOR_NEQ,
  OPERATOR_LT,
  OPERATOR_LTE,
  OPERATOR_GT,
  OPERATOR_GTE,
  OPERATOR_STRCONCAT,
  IDENTIFIER,
  TRY,
  EXPECT,
  INCLUDE_MACRO,
  ASSERT_MACRO,
  DEFINE_MACRO,
  UNDEFINE_MACRO,
  INFDEF_MACRO,
  INFNDEF_MACRO,
  ENDIF_MACRO,
};

#define NONE {.INT = 0}
struct PreprocessorToken {
  LocationRef location;
  union {
    int64_t INT;
    double FLOAT;
    InternedString STR;
    bool BOOL;
    Symbol SYMBOL;
    InternedString IDENTIFIER;
  } as;
  PreprocessorTokenTag tag;
};

inline PreprocessorToken build_pptoken_int(const LocationRef &location,
                                           int64_t value) {
  return PreprocessorToken{
      .location = location,
      .as = {.INT = value},
      .tag = PreprocessorTokenTag::INT,
  };
}

inline PreprocessorToken build_pptoken_float(const LocationRef &location,
                                             double value) {
  return PreprocessorToken{
      .location = location,
      .as = {.FLOAT = value},
      .tag = PreprocessorTokenTag::FLOAT,
  };
}

inline PreprocessorToken build_pptoken_string(const LocationRef &location,
                                              const InternedString &value) {
  return PreprocessorToken{
      .location = location,
      .as = {.STR = value},
      .tag = PreprocessorTokenTag::STRING,
  };
}

inline PreprocessorToken build_pptoken_bool(const LocationRef &location,
                                            bool value) {
  return PreprocessorToken{
      .location = location,
      .as = {.BOOL = value},
      .tag = PreprocessorTokenTag::BOOL,
  };
}

inline PreprocessorToken build_pptoken_symbol(const LocationRef &location,
                                              const Symbol &value) {
  return PreprocessorToken{
      .location = location,
      .as = {.SYMBOL = value},
      .tag = PreprocessorTokenTag::SYMBOL,
  };
}

inline PreprocessorToken build_pptoken_identifier(const LocationRef &location,
                                                  const InternedString &value) {
  return PreprocessorToken{
      .location = location,
      .as = {.IDENTIFIER = value},
      .tag = PreprocessorTokenTag::IDENTIFIER,
  };
}

inline PreprocessorToken build_pptoken(const LocationRef &location,
                                       PreprocessorTokenTag tag) {
  return PreprocessorToken{
      .location = location,
      .as = NONE,
      .tag = tag,
  };
}
