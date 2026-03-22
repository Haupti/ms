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
  auto pptokens = preprocessor_tokenize(&filename, code);
  IncludedModules mods;
  auto tokens = preprocess_pptokens(filename, &mods, pptokens, true);
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
                        "ADD\n"
                        "POP\n");
}
void test_op2(T *t) {
  string code = "1 + 2 * -1";
  string out = compile_and_show(code);
  t->assert_str_eq(out, "PUSH_INT 1\n"
                        "PUSH_INT 2\n"
                        "ADD\n"
                        "PUSH_INT 1\n"
                        "INVERT\n"
                        "MUL\n"
                        "POP\n");
}

void test_op3(T *t) {
  string code = "not a and b or not c";
  string out = compile_and_show(code);
  t->assert_str_eq(out, "LOAD a\n"
                        "NOT\n"
                        "PEEK_JMPIFN $AND_END_1\n"
                        "POP\n"
                        "LOAD b\n"
                        "LABEL $AND_END_1\n"
                        "PEEK_JMPIF $OR_END_0\n"
                        "POP\n"
                        "LOAD c\n"
                        "NOT\n"
                        "LABEL $OR_END_0\n"
                        "POP\n");
}

void test_if1(T *t) {
  string code = "if(#true) { 2+2 3+3 }";
  string out = compile_and_show(code);
  t->assert_str_eq(out, "PUSH_SYMBOL #true\n"
                        "JMPIFN $CONDITIONAL_END_2\n"
                        "SCOPE_START\n"
                        "PUSH_INT 2\n"
                        "PUSH_INT 2\n"
                        "ADD\n"
                        "POP\n"
                        "PUSH_INT 3\n"
                        "PUSH_INT 3\n"
                        "ADD\n"
                        "POP\n"
                        "SCOPE_END\n"
                        "LABEL $CONDITIONAL_END_2\n");
}

void test_if2(T *t) {
  string code = "if(#true) { #ok } else { #notok }";
  string out = compile_and_show(code);
  t->assert_str_eq(out, "PUSH_SYMBOL #true\n"
                        "JMPIFN $SKIP_LABEL_0_4\n"
                        "SCOPE_START\n"
                        "SCOPE_END\n"
                        "JMP $CONDITIONAL_END_3\n"
                        "LABEL $SKIP_LABEL_0_4\n"
                        "SCOPE_START\n"
                        "SCOPE_END\n"
                        "LABEL $CONDITIONAL_END_3\n");
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
                        "JMPIFN $SKIP_LABEL_0_6\n"
                        "SCOPE_START\n"
                        "SCOPE_END\n"
                        "JMP $CONDITIONAL_END_5\n"
                        "LABEL $SKIP_LABEL_0_6\n"
                        "PUSH_SYMBOL #false\n"
                        "JMPIFN $SKIP_LABEL_1_7\n"
                        "SCOPE_START\n"
                        "SCOPE_END\n"
                        "JMP $CONDITIONAL_END_5\n"
                        "LABEL $SKIP_LABEL_1_7\n"
                        "SCOPE_START\n"
                        "SCOPE_END\n"
                        "LABEL $CONDITIONAL_END_5\n");
}

void test_fn1(T *t) {
  string code = "function add(a,b) {\n"
                "return a + b\n"
                "}";
  string out = compile_and_show(code);
  t->assert_str_eq(out, "HALT\n"
                        "FUNCTION_START add locals:0\n"
                        "REGISTER_ARG a\n"
                        "REGISTER_ARG b\n"
                        "LOAD a\n"
                        "LOAD b\n"
                        "ADD\n"
                        "RETURN\n"
                        "PUSH_NONE\n"
                        "RETURN\n"
                        "FUNCTION_END\n");
}
void test_try(T *t) {
  string code = "function addone(b) {\n"
                "return try [b + 1]\n"
                "}";
  string out = compile_and_show(code);
  t->assert_str_eq(out, "HALT\n"
                        "FUNCTION_START addone locals:0\n"
                        "REGISTER_ARG b\n"
                        "LOAD b\n"
                        "PUSH_INT 1\n"
                        "ADD\n"
                        "DUP\n"
                        "TYPEOF\n"
                        "PUSH_SYMBOL #error\n"
                        "EQ\n"
                        "JMPIFN $TRY_8\n"
                        "RETURN\n"
                        "LABEL $TRY_8\n"
                        "RETURN\n"
                        "PUSH_NONE\n"
                        "RETURN\n"
                        "FUNCTION_END\n");
}
void test_expect(T *t) {
  string code = "function addone(a) {\n"
                "return expect [a + 1]\n"
                "}";
  string out = compile_and_show(code);
  t->assert_str_eq(out, "HALT\n"
                        "FUNCTION_START addone locals:0\n"
                        "REGISTER_ARG a\n"
                        "LOAD a\n"
                        "PUSH_INT 1\n"
                        "ADD\n"
                        "DUP\n"
                        "TYPEOF\n"
                        "PUSH_SYMBOL #error\n"
                        "EQ\n"
                        "JMPIFN $EXPECT_9\n"
                        "PUSH_ALLOC_STRING expected non-error value\n"
                        "VMCALL panic\n"
                        "LABEL $EXPECT_9\n"
                        "RETURN\n"
                        "PUSH_NONE\n"
                        "RETURN\n"
                        "FUNCTION_END\n");
}

void test_ordering(T *t) {
  string code = "let x =1 \n"
                "function void(a) {}\n";
  string out = compile_and_show(code);
  t->assert_str_eq(out, "PUSH_INT 1\n"
                        "STORE_NEW x\n"
                        "HALT\n"
                        "FUNCTION_START void locals:0\n"
                        "REGISTER_ARG a\n"
                        "PUSH_NONE\n"
                        "RETURN\n"
                        "FUNCTION_END\n");
}
void test_for_loop(T *t) {
  string code = "for x in somelist {\n"
                "#noop\n"
                "}\n";
  string out = compile_and_show(code);
  t->assert_str_eq(out, "LOAD somelist\n"
                        "LABEL $START_LOOP_10\n"
                        "ITER_FOREACH $END_LOOP_11\n"
                        "SCOPE_START\n"
                        "STORE_NEW x\n"
                        "JMP $START_LOOP_10\n"
                        "LABEL $END_LOOP_11\n"
                        "SCOPE_END\n");
}
void test_for_loop_continue(T *t) {
  string code = "for x in somelist {\n"
                "continue\n"
                "}\n";
  string out = compile_and_show(code);
  t->assert_str_eq(out, "LOAD somelist\n"
                        "LABEL $START_LOOP_12\n"
                        "ITER_FOREACH $END_LOOP_13\n"
                        "SCOPE_START\n"
                        "STORE_NEW x\n"
                        "JMP $START_LOOP_12\n"
                        "JMP $START_LOOP_12\n"
                        "LABEL $END_LOOP_13\n"
                        "SCOPE_END\n");
}
void test_for_loop_break(T *t) {
  string code = "for x in somelist {\n"
                "break\n"
                "}\n";
  string out = compile_and_show(code);
  t->assert_str_eq(out, "LOAD somelist\n"
                        "LABEL $START_LOOP_14\n"
                        "ITER_FOREACH $END_LOOP_15\n"
                        "SCOPE_START\n"
                        "STORE_NEW x\n"
                        "JMP $END_LOOP_15\n"
                        "JMP $START_LOOP_14\n"
                        "LABEL $END_LOOP_15\n"
                        "SCOPE_END\n");
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
  t.test("function 1", test_fn1);
  t.test("try", test_try);
  t.test("expect", test_expect);
  t.test("functions come after rest", test_ordering);
  t.test("for loop", test_for_loop);
  t.test("for loop continue", test_for_loop_continue);
  t.test("for loop break", test_for_loop_break);
  return 0;
}
