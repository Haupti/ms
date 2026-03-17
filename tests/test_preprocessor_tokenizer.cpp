#include "../lib/asap/t.hpp"
#include "../lib/asap/util.hpp"
#include "../src/preprocessor/preprocessor_tokenize.hpp"
#include "testutil_token_to_string.hpp"

using namespace std;
namespace {

static string test_filename = "test.msl";
void test_stuff(T *t) {
  vector<PreprocessorToken> tokens = preprocessor_tokenize(
      &test_filename, "1 1.1 1.1.print() true #true #false #none #1 #_1 "
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
      &test_filename,
      "-fset TESTRUN -funset TESTRUN -iffset TESTRUN print(x) -endif");

  vector<string> strs;
  strs.reserve(tokens.size());
  for (auto t : tokens) {
    strs.push_back(pptoken_to_string(t));
  }
  t->assert_str_eq(join(strs, " "),
                   "MACRO(-fset) TESTRUN MACRO(-funset) TESTRUN MACRO(-iffset) "
                   "TESTRUN print ( x ) MACRO(-endif)");
}

} // namespace

int main() {
  T t("Preprocessor tokenization");
  t.test("stuff", test_stuff);
  t.test("macros", test_fset_and_such);
  return 0;
}
