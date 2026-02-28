#include "../lib/asap/t.hpp"
#include "../src/ir/compile_ir.hpp"
#include "../src/parser/node.hpp"
#include "../src/parser/parser.hpp"
#include "../src/preprocessor/preprocess.hpp"
#include "../src/preprocessor/preprocessor_tokenize.hpp"
#include "testutil_ir_to_string.hpp"
using namespace std;
namespace {
static string filename = "test.msl";
string compile_and_show(string code) {
  auto pptokens = preprocessor_tokenize(filename, code);
  IncludedModules mods;
  auto tokens = preprocess_pptokens(filename, &mods, pptokens);
  nodes n = parse(tokens);
  vector<IRInstr> ir = compile_ir(n);
  string out = "";
  for (auto i : ir) {
    out += ir_to_string(i);
    out += "\n";
  }
  return out;
}
void test_op1(T *t) {
  string code = "1 + 2";
  string out = compile_and_show(code);
  t->assert_str_eq(out, "[test.msl/0/2] PUSH_INT 1\n"
                        "[test.msl/0/6] PUSH_INT 2\n"
                        "[test.msl/0/4] ADD\n");
}

} // namespace
int main() {
  T t("IR Compiler");
  t.test("operator expression", test_op1);
  return 0;
}
