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
  t->assert_str_eq(out, "PUSH_INT 1\n"
                        "PUSH_INT 2\n"
                        "ADD\n");
}
void test_op2(T *t) {
  string code = "1 + 2 * -1";
  string out = compile_and_show(code);
  t->assert_str_eq(out, "PUSH_INT 1\n"
                        "PUSH_INT 2\n"
                        "ADD\n"
                        "PUSH_INT 1\n"
                        "INVERT\n"
                        "MUL\n");
}

void test_op3(T *t) {
  string code = "not a and b or not c";
  string out = compile_and_show(code);
  t->assert_str_eq(out, "LOAD a\n"
                        "NOT\n"
                        "LOAD b\n"
                        "AND\n"
                        "LOAD c\n"
                        "NOT\n"
                        "OR\n");
}
void test_if1(T *t) {
  string code = "if(#true) { 2+2 3+3 }";
  string out = compile_and_show(code);
  t->assert_str_eq(out, "PUSH_SYMBOL #true\n"
                        "JMPIFN conditional_end_0\n"
                        "PUSH_INT 2\n"
                        "PUSH_INT 2\n"
                        "ADD\n"
                        "PUSH_INT 3\n"
                        "PUSH_INT 3\n"
                        "ADD\n"
                        "LABEL conditional_end_0\n");
}

void test_if2(T *t) {
  string code = "if(#true) { #ok } else { #notok }";
  string out = compile_and_show(code);
  t->assert_str_eq(out, "PUSH_SYMBOL #true\n"
                        "JMPIFN skip_label_0_2\n"
                        "PUSH_SYMBOL #ok\n"
                        "LABEL skip_label_0_2\n"
                        "PUSH_SYMBOL #notok\n"
                        "LABEL conditional_end_1\n");
}

void test_if3(T *t) {
  string code = "if(#true) { \n"
                "   #ifcase \n"
                "} elif (#false) {\n"
                "   #elifcase \n"
                "} else {\n"
                "   #elsecase }";
  string out = compile_and_show(code);
  t->assert_str_eq(out, "PUSH_SYMBOL #true\n"
                        "JMPIFN skip_label_0_4\n"
                        "PUSH_SYMBOL #ifcase\n"
                        "LABEL skip_label_0_4\n"
                        "PUSH_SYMBOL #false\n"
                        "JMPIFN skip_label_1_5\n"
                        "PUSH_SYMBOL #elifcase\n"
                        "LABEL skip_label_1_5\n"
                        "PUSH_SYMBOL #elsecase\n"
                        "LABEL conditional_end_3\n");
}

} // namespace
int main() {
  // the order here matters because the label index is static
  // and will increase between tests
  T t("IR Compiler");
  t.test("operator expression 1", test_op1);
  t.test("operator expression 2", test_op2);
  t.test("operator expression 3", test_op3);
  t.test("if condition 1", test_if1);
  t.test("if condition 2", test_if2);
  t.test("if condition 3", test_if3);
  return 0;
}
