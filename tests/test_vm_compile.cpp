#include "../lib/asap/t.hpp"
#include "../src/instr/compile_to_vm.hpp"
#include "../src/ir/compile_ir.hpp"
#include "../src/parser/node.hpp"
#include "../src/parser/parser.hpp"
#include "../src/preprocessor/preprocess.hpp"
#include "../src/preprocessor/preprocessor_tokenize.hpp"
#include "../src/vm/vm_instr.hpp"
using namespace std;
namespace {
static string filename = "test.msl";
string compile_and_show(string code) {
  auto pptokens = preprocessor_tokenize(filename, code);
  IncludedModules mods;
  auto tokens = preprocess_pptokens(filename, &mods, pptokens);
  nodes n = parse(tokens);
  vector<IRInstr> ir = compile_ir(n);
  vector<VMInstr> vm = compile_to_vm(ir);
  string out = "";
  for (uint64_t pos = 0; pos < vm.size(); ++pos) {
    out += vminstr_to_string(pos, vm.at(pos));
    out += "\n";
  }
  return out;
}
void test_var1(T *t) {
  string code = "let x = 1 + 2";
  string out = compile_and_show(code);
  t->assert_str_eq(out, "0 PROGRAM_INIT 1\n"
                        "1 PUSH_INT 1\n"
                        "2 PUSH_INT 2\n"
                        "3 ADD\n"
                        "4 STORE_GLOBAL mem(0)\n"
                        "5 HALT\n");
}
void test_op1(T *t) {
  string code = "1 + 2";
  string out = compile_and_show(code);
  t->assert_str_eq(out, "0 PROGRAM_INIT 0\n"
                        "1 PUSH_INT 1\n"
                        "2 PUSH_INT 2\n"
                        "3 ADD\n"
                        "4 POP\n"
                        "5 HALT\n");
}

void test_op2(T *t) {
  string code = "1 + 2 * -1";
  string out = compile_and_show(code);
  t->assert_str_eq(out, "0 PROGRAM_INIT 0\n"
                        "1 PUSH_INT 1\n"
                        "2 PUSH_INT 2\n"
                        "3 ADD\n"
                        "4 PUSH_INT 1\n"
                        "5 INVERT\n"
                        "6 MUL\n"
                        "7 POP\n"
                        "8 HALT\n");
}

void test_op3(T *t) {
  string code = "let a = #false\n"
                "let b = #true\n"
                "let c = #false\n"
                "not a and b or not c";
  string out = compile_and_show(code);
  t->assert_str_eq(out, "0 PROGRAM_INIT 3\n"
                        "1 PUSH_SYMBOL #false\n"
                        "2 STORE_GLOBAL mem(0)\n"
                        "3 PUSH_SYMBOL #true\n"
                        "4 STORE_GLOBAL mem(1)\n"
                        "5 PUSH_SYMBOL #false\n"
                        "6 STORE_GLOBAL mem(2)\n"
                        "7 LOAD_GLOBAL mem(0)\n"
                        "8 NOT\n"
                        "9 PEEK_JMPIFN addr(12)\n"
                        "10 POP\n"
                        "11 LOAD_GLOBAL mem(1)\n"
                        "12 PEEK_JMPIF addr(16)\n"
                        "13 POP\n"
                        "14 LOAD_GLOBAL mem(2)\n"
                        "15 NOT\n"
                        "16 POP\n"
                        "17 HALT\n");
}

void test_if1(T *t) {
  string code = "if(#true) {\n"
                "  let x = 2 + 2\n"
                "  set x = 3+3\n"
                "}";
  string out = compile_and_show(code);
  t->assert_str_eq(out, "0 PROGRAM_INIT 1\n"
                        "1 PUSH_SYMBOL #true\n"
                        "2 JMPIFN addr(11)\n"
                        "3 PUSH_INT 2\n"
                        "4 PUSH_INT 2\n"
                        "5 ADD\n"
                        "6 STORE mem(0)\n"
                        "7 PUSH_INT 3\n"
                        "8 PUSH_INT 3\n"
                        "9 ADD\n"
                        "10 STORE mem(0)\n"
                        "11 HALT\n");
}

void test_if2(T *t) {
  string code = "let x = 1\n"
                "if(#true) { set x = 2 } else { set x = 3 }";
  string out = compile_and_show(code);
  t->assert_str_eq(out, "0 PROGRAM_INIT 1\n"
                        "1 PUSH_INT 1\n"
                        "2 STORE_GLOBAL mem(0)\n"
                        "3 PUSH_SYMBOL #true\n"
                        "4 JMPIFN addr(8)\n"
                        "5 PUSH_INT 2\n"
                        "6 STORE_GLOBAL mem(0)\n"
                        "7 JMP addr(10)\n"
                        "8 PUSH_INT 3\n"
                        "9 STORE_GLOBAL mem(0)\n"
                        "10 HALT\n");
}

void test_if3(T *t) {
  string code = "if(#true) { \n"
                "   #ifcase \n"
                "} elif (#false) {\n"
                "   #elifcase \n"
                "} else {\n"
                "   #elsecase }";
  string out = compile_and_show(code);
  t->assert_str_eq(out, "0 PROGRAM_INIT 0\n"
                        "1 PUSH_SYMBOL #true\n"
                        "2 JMPIFN addr(4)\n"
                        "3 JMP addr(7)\n"
                        "4 PUSH_SYMBOL #false\n"
                        "5 JMPIFN addr(7)\n"
                        "6 JMP addr(7)\n"
                        "7 HALT\n");
}

void test_fn1(T *t) {
  string code = "function add(a,b) {\n"
                "return a + b\n"
                "}";
  string out = compile_and_show(code);
  t->assert_str_eq(out, "0 PROGRAM_INIT 0\n"
                        "1 HALT\n"
                        "2 INIT_FRAME 0 2\n"
                        "3 LOAD mem(0)\n"
                        "4 LOAD mem(1)\n"
                        "5 ADD\n"
                        "6 RETURN\n"
                        "7 PUSH_NONE \n"
                        "8 RETURN\n"
                        "9 HALT\n");
}

void test_try(T *t) {
  string code = "function addone(b) {\n"
                "return try [b + 1]\n"
                "}";
  string out = compile_and_show(code);
  t->assert_str_eq(out, "0 PROGRAM_INIT 0\n"
                        "1 HALT\n"
                        "2 INIT_FRAME 0 1\n"
                        "3 LOAD mem(0)\n"
                        "4 PUSH_INT 1\n"
                        "5 ADD\n"
                        "6 DUP\n"
                        "7 TYPEOF\n"
                        "8 PUSH_SYMBOL #error\n"
                        "9 EQ\n"
                        "10 JMPIFN addr(12)\n"
                        "11 RETURN\n"
                        "12 RETURN\n"
                        "13 PUSH_NONE \n"
                        "14 RETURN\n"
                        "15 HALT\n");
}

void test_expect(T *t) {
  string code = "function addone(a) {\n"
                "return expect [a + 1]\n"
                "}";
  string out = compile_and_show(code);
  t->assert_str_eq(out, "0 PROGRAM_INIT 0\n"
                        "1 HALT\n"
                        "2 INIT_FRAME 0 1\n"
                        "3 LOAD mem(0)\n"
                        "4 PUSH_INT 1\n"
                        "5 ADD\n"
                        "6 DUP\n"
                        "7 TYPEOF\n"
                        "8 PUSH_SYMBOL #error\n"
                        "9 EQ\n"
                        "10 JMPIFN addr(13)\n"
                        "11 PUSH_ALLOC_STRING 'expected non-error value'\n"
                        "12 VMCALL panic 1\n"
                        "13 RETURN\n"
                        "14 PUSH_NONE \n"
                        "15 RETURN\n"
                        "16 HALT\n");
}

void test_ordering(T *t) {
  string code = "let x =1 \n"
                "function void(a) {}\n";
  string out = compile_and_show(code);
  t->assert_str_eq(out, "0 PROGRAM_INIT 1\n"
                        "1 PUSH_INT 1\n"
                        "2 STORE_GLOBAL mem(0)\n"
                        "3 HALT\n"
                        "4 INIT_FRAME 0 1\n"
                        "5 PUSH_NONE \n"
                        "6 RETURN\n"
                        "7 HALT\n");
}
void test_complex(T *t) {
  string code = "let x = 1\n"
                "function void(a) {\n"
                "  return x + a\n"
                "}\n"
                "if (x == 1) {\n"
                "  let x = 2\n"
                "  set x = 3\n"
                "}\n";
  string out = compile_and_show(code);
  t->assert_str_eq(out, "0 PROGRAM_INIT 2\n"
                        "1 PUSH_INT 1\n"
                        "2 STORE_GLOBAL mem(0)\n"
                        "3 LOAD_GLOBAL mem(0)\n"
                        "4 PUSH_INT 1\n"
                        "5 EQ\n"
                        "6 JMPIFN addr(11)\n"
                        "7 PUSH_INT 2\n"
                        "8 STORE mem(1)\n"
                        "9 PUSH_INT 3\n"
                        "10 STORE mem(1)\n"
                        "11 HALT\n"
                        "12 INIT_FRAME 0 1\n"
                        "13 LOAD_GLOBAL mem(0)\n"
                        "14 LOAD mem(0)\n"
                        "15 ADD\n"
                        "16 RETURN\n"
                        "17 PUSH_NONE \n"
                        "18 RETURN\n"
                        "19 HALT\n");
}
void test_fn_call(T *t) {
  string code = "function add(a,b) { return a + b }"
                "add(1,2)\n";
  string out = compile_and_show(code);
  t->assert_str_eq(out, "0 PROGRAM_INIT 0\n"
                        "1 PUSH_INT 1\n"
                        "2 PUSH_INT 2\n"
                        "3 CALL addr(6)\n"
                        "4 POP\n"
                        "5 HALT\n"
                        "6 INIT_FRAME 0 2\n"
                        "7 LOAD mem(0)\n"
                        "8 LOAD mem(1)\n"
                        "9 ADD\n"
                        "10 RETURN\n"
                        "11 PUSH_NONE \n"
                        "12 RETURN\n"
                        "13 HALT\n");
}
void test_for_loop(T *t) {
  string code = "let somelist = list(1)\n"
                "for x in somelist {"
                "#noop\n"
                "}\n";
  string out = compile_and_show(code);
  t->assert_str_eq(out, "0 PROGRAM_INIT 2\n"
                        "1 PUSH_INT 1\n"
                        "2 PUSH_INT 1\n"
                        "3 VMCALL list 1\n"
                        "4 STORE_GLOBAL mem(0)\n"
                        "5 LOAD_GLOBAL mem(0)\n"
                        "6 ITER_FOREACH addr(9)\n"
                        "7 STORE mem(1)\n"
                        "8 JMP addr(6)\n"
                        "9 POP\n"
                        "10 HALT\n");
}

} // namespace
int main() {
  // the order here matters because the label index is static
  // and will increase between tests
  T t("VM Compiler");
  t.test("var let/set 1", test_var1);
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
  t.test("complex", test_complex);
  t.test("fn call", test_fn_call);
  t.test("for loop", test_for_loop);
  return 0;
}
