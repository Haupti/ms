#include "../lib/asap/t.hpp"
#include "../lib/asap/util.hpp"
#include "../src/preprocessor/preprocessor_tokenize.hpp"
#include "../src/symbol.hpp"

using namespace std;
namespace {
string pptoken_to_string(const PreprocessorToken &token) {
  switch (token.tag) {
  case PpTokenTag::INT:
    return to_string(token.as.INT);
  case PpTokenTag::FLOAT:
    return to_string(token.as.FLOAT);
  case PpTokenTag::SYMBOL:
    return resolve_symbol(token.as.SYMBOL);
  case PpTokenTag::STRING:
    return "\"" + resolve_interned_string(token.as.STR) + "\"";
  case PpTokenTag::LET:
    return "let";
  case PpTokenTag::SET:
    return "set";
  case PpTokenTag::AT:
    return "at";
  case PpTokenTag::IF:
    return "if";
  case PpTokenTag::ELIF:
    return "elif";
  case PpTokenTag::ELSE:
    return "else";
  case PpTokenTag::FUNCTION:
    return "function";
  case PpTokenTag::TRY:
    return "try";
  case PpTokenTag::EXPECT:
    return "expect";
  case PpTokenTag::ASSIGN:
    return "=";
  case PpTokenTag::BROPEN:
    return "(";
  case PpTokenTag::BRCLOSE:
    return ")";
  case PpTokenTag::CURLOPEN:
    return "{";
  case PpTokenTag::CURLCLOSE:
    return "}";
  case PpTokenTag::COMMA:
    return ",";
  case PpTokenTag::DOT:
    return ".";
  case PpTokenTag::BAR:
    return "|";
  case PpTokenTag::OP_ADD:
    return "+";
  case PpTokenTag::OP_SUB:
    return "-";
  case PpTokenTag::OP_MUL:
    return "*";
  case PpTokenTag::OP_DIV:
    return "/";
  case PpTokenTag::OP_MOD:
    return "mod";
  case PpTokenTag::OP_EQ:
    return "==";
  case PpTokenTag::OP_NEQ:
    return "!=";
  case PpTokenTag::OP_NOT:
    return "not";
  case PpTokenTag::OP_OR:
    return "or";
  case PpTokenTag::OP_AND:
    return "and";
  case PpTokenTag::OPERATOR_LT:
    return "<";
  case PpTokenTag::OPERATOR_LTE:
    return "<=";
  case PpTokenTag::OPERATOR_GT:
    return ">";
  case PpTokenTag::OPERATOR_GTE:
    return ">=";
  case PpTokenTag::OPERATOR_STRCONCAT:
    return "<>";
  case PpTokenTag::IDENTIFIER:
    return resolve_interned_string(token.as.IDENTIFIER);
  case PpTokenTag::INCLUDE_MACRO:
    return "MACRO(-include)";
  case PpTokenTag::DEFINE_MACRO:
    return "MACRO(-define)";
  case PpTokenTag::FSET_MACRO:
    return "MACRO(-fset)";
  case PpTokenTag::FUNSET_MACRO:
    return "MACRO(-funset)";
  case PpTokenTag::IFFSET_MACRO:
    return "MACRO(-iffset)";
  case PpTokenTag::IFNFSET_MACRO:
    return "MACRO(-ifnfset)";
  case PpTokenTag::ENDIF_MACRO:
    return "MACRO(-endif)";
  }
}

void test_stuff(T *t) {
  vector<PreprocessorToken> tokens = preprocessor_tokenize(
      "test.msl",
      "1 1.1 1.1.print() true #true #false #none #1 #_1 "
      "\"hello \n world\""
      "if ( #true ) { + - == let x = 0} "
      "fn(1,2,3) fn:name_1(_aaa_0_AZ) try expect");

  vector<string> strs;
  strs.reserve(tokens.size());
  for (auto t : tokens) {
    strs.push_back(pptoken_to_string(t));
  }
  t->assert_str_eq(
      join(strs, " "),
      "1 1.100000 1.100000 . print ( ) true #true #false #none #1 #_1 \"hello "
      "\n"
      " world\" if ( #true ) { + - == let x = 0 } fn ( 1 , 2 , 3 ) fn:name_1 ( "
      "_aaa_0_AZ ) try expect");
}

void test_fset_and_such(T *t) {
  vector<PreprocessorToken> tokens = preprocessor_tokenize(
      "test.msl", "-fset TESTRUN -funset TESTRUN -iffset TESTRUN print(x) -endif");

  vector<string> strs;
  strs.reserve(tokens.size());
  for (auto t : tokens) {
    strs.push_back(pptoken_to_string(t));
  }
  t->assert_str_eq(
      join(strs, " "),
      "MACRO(-fset) TESTRUN MACRO(-funset) TESTRUN MACRO(-iffset) TESTRUN print ( x ) MACRO(-endif)");
}

} // namespace

int main() {
  T t("Preprocessor tokenization");
  t.test("stuff", test_stuff);
  t.test("macros", test_fset_and_such);
  return 0;
}
