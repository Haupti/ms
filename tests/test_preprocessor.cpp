#include "../lib/asap/t.hpp"
#include "../lib/asap/util.hpp"
#include "../src/preprocessor/preprocess.hpp"
#include "../src/preprocessor/preprocessor_tokenize.hpp"
#include "../src/preprocessor/token.hpp"

using namespace std;
namespace {
static string test_filename = "test.msl";
static IncludedModules includes;

void test_no_flag(T *t) {
  vector<PreprocessorToken> pptokens =
      preprocessor_tokenize(&test_filename, "-fset TESTRUN"
                                            "-funset TESTRUN"
                                            "-iffset TESTRUN"
                                            "print(\"testrun\") "
                                            "-endif x");
  vector<Token> tokens =
      preprocess_pptokens(test_filename, &includes, pptokens);

  vector<string> strs;
  strs.reserve(tokens.size());
  for (auto t : tokens) {
    strs.push_back(token_to_string(t));
  }
  t->assert_str_eq(join(strs, " "), "x");
}
void test_flag(T *t) {
  vector<PreprocessorToken> pptokens = preprocessor_tokenize(
      &test_filename, "-ifnfset TESTRUN "
                      "#nope print(\"testrun\") -endif print(x)");
  vector<Token> tokens =
      preprocess_pptokens(test_filename, &includes, pptokens);

  vector<string> strs;
  strs.reserve(tokens.size());
  for (auto t : tokens) {
    strs.push_back(token_to_string(t));
  }
  t->assert_str_eq(join(strs, " "), "#nope print ( \"testrun\" ) print ( x )");
}
void test_nested_if(T *t) {
  vector<PreprocessorToken> pptokens =
      preprocessor_tokenize(&test_filename, "-fset A "
                                            "-iffset A "
                                            "  something"
                                            "  -ifnfset B "
                                            "    inner "
                                            "    inner2 "
                                            "  -endif "
                                            "-endif "
                                            "-iffset B "
                                            "  -iffset A "
                                            "    nope "
                                            "  -endif "
                                            "-endif "
                                            "outer");
  vector<Token> tokens =
      preprocess_pptokens(test_filename, &includes, pptokens);

  vector<string> strs;
  strs.reserve(tokens.size());
  for (auto t : tokens) {
    strs.push_back(token_to_string(t));
  }
  t->assert_str_eq(join(strs, " "), "something inner inner2 outer");
}
} // namespace

int main() {
  T t("Preprocessor");
  t.test("skip stuff", test_no_flag);
  t.test("put in stuff", test_flag);
  t.test("nested if", test_nested_if);
  return 0;
}
