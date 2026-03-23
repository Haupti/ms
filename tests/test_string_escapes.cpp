#include "../lib/asap/t.hpp"
#include "../src/preprocessor/preprocessor_tokenize.hpp"
#include "testutil_token_to_string.hpp"

using namespace std;
namespace {

static string test_filename = "test.msl";
void test_escapes(T *t) {
  vector<PreprocessorToken> tokens = preprocessor_tokenize(
      &test_filename, R"msl("\n\t\r\"\\")msl");

  t->assert(tokens.size() == 1);
  string s = pptoken_to_string(tokens[0]);
  
  string expected = "\"";
  expected += '\n';
  expected += '\t';
  expected += '\r';
  expected += '"';
  expected += '\\';
  expected += '"';

  t->assert_str_eq(s, expected);
}

void test_invalid_escape(T *t) {
  t->assert_throws([&]() {
    preprocessor_tokenize(&test_filename, R"msl("\x")msl");
  });
}

void test_unclosed_string(T *t) {
  t->assert_throws([&]() {
    preprocessor_tokenize(&test_filename, R"msl("hello)msl");
  });
}

} // namespace

int main() {
  T t("String Escapes");
  t.test("escapes", test_escapes);
  t.test("invalid escape", test_invalid_escape);
  t.test("unclosed string", test_unclosed_string);
  return 0;
}
