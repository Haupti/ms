#include "../lib/asap/t.hpp"
#include "../src/preprocessor/preprocessor_tokenize.hpp"
#include "../src/symbol.hpp"
#include <sstream>

using namespace std;
namespace {
string pptoken_to_string(const PreprocessorToken &token) {
  switch (token.tag) {
  case PpTokenTag::INT:
    return to_string(token.as.INT);
  case PpTokenTag::FLOAT:
    return to_string(token.as.FLOAT);
  case PpTokenTag::SYMBOL:
    return "S-" + resolve_symbol(token.as.SYMBOL);
  case PpTokenTag::STRING:
    return "\"" + resolve_interned_string(token.as.STR) + "\"";
  case PpTokenTag::LET:
    return "K-let";
  case PpTokenTag::SET:
    return "K-set";
  case PpTokenTag::AT:
    return "K-at";
  case PpTokenTag::IF:
    return "K-if";
  case PpTokenTag::ELIF:
    return "K-elif";
  case PpTokenTag::ELSE:
    return "K-else";
  case PpTokenTag::FUNCTION:
    return "K-function";
  case PpTokenTag::TRY:
    return "K-try";
  case PpTokenTag::EXPECT:
    return "K-expect";
  case PpTokenTag::ASSIGN:
    return "K-=";
  case PpTokenTag::BROPEN:
    return "K-(";
  case PpTokenTag::BRCLOSE:
    return "K-)";
  case PpTokenTag::CURLOPEN:
    return "K-{";
  case PpTokenTag::CURLCLOSE:
    return "K-}";
  case PpTokenTag::COMMA:
    return "K-,";
  case PpTokenTag::DOT:
    return "O-.";
  case PpTokenTag::BAR:
    return "O-|";
  case PpTokenTag::OP_ADD:
    return "O-+";
  case PpTokenTag::OP_SUB:
    return "O--";
  case PpTokenTag::OP_MUL:
    return "O-*";
  case PpTokenTag::OP_DIV:
    return "O-/";
  case PpTokenTag::OP_MOD:
    return "O-mod";
  case PpTokenTag::OP_EQ:
    return "O-==";
  case PpTokenTag::OP_NEQ:
    return "O-!=";
  case PpTokenTag::OP_NOT:
    return "O-not";
  case PpTokenTag::OP_OR:
    return "O-or";
  case PpTokenTag::OP_AND:
    return "O-and";
  case PpTokenTag::OPERATOR_LT:
    return "O-<";
  case PpTokenTag::OPERATOR_LTE:
    return "O-<=";
  case PpTokenTag::OPERATOR_GT:
    return "O->";
  case PpTokenTag::OPERATOR_GTE:
    return "O->=";
  case PpTokenTag::OPERATOR_STRCONCAT:
    return "O-<>";
  case PpTokenTag::IDENTIFIER:
    return "ID-" + resolve_interned_string(token.as.IDENTIFIER);
  case PpTokenTag::INCLUDE_MACRO:
    return "M-include";
  case PpTokenTag::ASSERT_MACRO:
    return "M-assert";
  case PpTokenTag::DEFINE_MACRO:
    return "M-define";
  case PpTokenTag::UNDEFINE_MACRO:
    return "M-undefine";
  case PpTokenTag::IFDEF_MACRO:
    return "M-ifdef";
  case PpTokenTag::IFNOTDEF_MACRO:
    return "M-ifnotdef";
  case PpTokenTag::ENDIF_MACRO:
    return "M-endif";
  }
}
} // namespace

void test_example(T *t) {
  vector<PreprocessorToken> tokens = preprocessor_tokenize(
      "test.msl",
      "1 1.1 1.1.print() true #true #false #none #1 #_1 "
      "\"hello \n world\""
      "if ( #true ) { + - == let x = 0} "
      "fn(1,2,3) fn:name_1(_aaa_0_AZ) try expect -define -endif -assert(x)");

  stringstream ss;
  for (auto t : tokens) {
    ss << pptoken_to_string(t) << " ";
  }
  t->assert_str_eq(ss.str(),
                   "1 1.100000 1.100000 O-. ID-print K-( K-) ID-true S-#true "
                   "S-#false S-#none S-#1 S-#_1 \"hello "
                   "\n world\" K-if K-( S-#true K-) K-{ O-+ O-- O-== K-let ID-x "
                   "K-= 0 K-} ID-fn K-( 1 K-, 2 K-, 3 K-) ID-fn:name_1 "
                   "K-( ID-_aaa_0_AZ K-) K-try K-expect M-define M-endif "
                   "M-assert K-( ID-x K-) ");
}

int main() {
  T t("Preprocessor tokenization");
  t.test("stuff", test_example);
  return 0;
}
