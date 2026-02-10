#include "../lib/asap/t.hpp"
#include "../lib/asap/util.hpp"
#include "../src/preprocessor/preprocess.hpp"
#include "../src/preprocessor/preprocessor_tokenize.hpp"
#include "../src/preprocessor/token.hpp"
#include "../src/symbol.hpp"

using namespace std;
namespace {
string token_to_string(const Token &token) {
  switch (token.tag) {
  case TokenTag::INT:
    return to_string(token.as.INT);
  case TokenTag::FLOAT:
    return to_string(token.as.FLOAT);
  case TokenTag::SYMBOL:
    return resolve_symbol(token.as.SYMBOL);
  case TokenTag::STRING:
    return "\"" + resolve_interned_string(token.as.STR) + "\"";
  case TokenTag::LET:
    return "let";
  case TokenTag::SET:
    return "set";
  case TokenTag::AT:
    return "at";
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
  }
}

void test_no_flag(T *t) {
  vector<PreprocessorToken> pptokens =
      preprocessor_tokenize("test.msl", "-fset TESTRUN"
                                        "-funset TESTRUN"
                                        "-iffset TESTRUN"
                                        "print(\"testrun\") "
                                        "-endif x");
  vector<Token> tokens = preprocess_pptokens(pptokens);

  vector<string> strs;
  strs.reserve(tokens.size());
  for (auto t : tokens) {
    strs.push_back(token_to_string(t));
  }
  t->assert_str_eq(join(strs, " "), "x");
}
void test_flag(T *t) {
  vector<PreprocessorToken> pptokens = preprocessor_tokenize(
      "test.msl", "-ifnfset TESTRUN "
                  "#nope print(\"testrun\") -endif print(x)");
  vector<Token> tokens = preprocess_pptokens(pptokens);

  vector<string> strs;
  strs.reserve(tokens.size());
  for (auto t : tokens) {
    strs.push_back(token_to_string(t));
  }
  t->assert_str_eq(join(strs, " "), "#nope print ( \"testrun\" ) print ( x )");
}
} // namespace

int main() {
  T t("Preprocessor");
  t.test("skip stuff", test_no_flag);
  t.test("put in stuff", test_flag);
  return 0;
}
