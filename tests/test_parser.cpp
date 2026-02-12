#include "../lib/asap/t.hpp"
#include "../src/parser/node.hpp"
#include "../src/parser/parser.hpp"
#include "../src/preprocessor/preprocess.hpp"
#include "../src/preprocessor/preprocessor_tokenize.hpp"
#include "testutil_node_to_string.hpp"
using namespace std;
namespace {
static string filename = "test.msl";
string parse_and_show(string code) {
  auto pptokens = preprocessor_tokenize(filename, code);
  IncludedModules mods;
  auto tokens = preprocess_pptokens(filename, &mods, pptokens);
  auto n = parse(tokens);
  string out = "";
  for (auto node : n.elements) {
    out += show_node(node);
    out += "-----------------------\n";
  }
  return out;
}
void test_literal(T *t) {
  string code = "4 \"hello\" #to #mom 3.5";
  string out = parse_and_show(code);
  t->assert_str_eq(out, "Node  NIL:\n"
                        "    Start = NIL\n"
                        "    First_Child = 0\n"
                        "    Next_Child = 0\n"
                        "    Next_Sibling = 0\n"
                        "}\n"
                        "-----------------------\n"
                        "Node  LITERAL_INT = 4:\n"
                        "    Start = test.msl/0/2\n"
                        "    First_Child = 0\n"
                        "    Next_Child = 0\n"
                        "    Next_Sibling = 2\n"
                        "}\n"
                        "-----------------------\n"
                        "Node  LITERAL_STRING = 'hello':\n"
                        "    Start = test.msl/0/4\n"
                        "    First_Child = 0\n"
                        "    Next_Child = 0\n"
                        "    Next_Sibling = 3\n"
                        "}\n"
                        "-----------------------\n"
                        "Node  LITERAL_SYMBOL = #to:\n"
                        "    Start = test.msl/0/12\n"
                        "    First_Child = 0\n"
                        "    Next_Child = 0\n"
                        "    Next_Sibling = 4\n"
                        "}\n"
                        "-----------------------\n"
                        "Node  LITERAL_SYMBOL = #mom:\n"
                        "    Start = test.msl/0/16\n"
                        "    First_Child = 0\n"
                        "    Next_Child = 0\n"
                        "    Next_Sibling = 5\n"
                        "}\n"
                        "-----------------------\n"
                        "Node  LITERAL_FLOAT = 3.500000:\n"
                        "    Start = test.msl/0/21\n"
                        "    First_Child = 0\n"
                        "    Next_Child = 0\n"
                        "    Next_Sibling = 0\n"
                        "}\n"
                        "-----------------------\n"

  );
}
void test_var_def(T *t) {
  string code = "let x = 1 5";
  string out = parse_and_show(code);
  t->assert_str_eq(out, "Node  NIL:\n"
                        "    Start = NIL\n"
                        "    First_Child = 0\n"
                        "    Next_Child = 0\n"
                        "    Next_Sibling = 0\n"
                        "}\n"
                        "-----------------------\n"
                        "Node  VAR_DEF = x:\n"
                        "    Start = test.msl/0/2\n"
                        "    First_Child = 2\n"
                        "    Next_Child = 0\n"
                        "    Next_Sibling = 3\n"
                        "}\n"
                        "-----------------------\n"
                        "Node  LITERAL_INT = 1:\n"
                        "    Start = test.msl/0/10\n"
                        "    First_Child = 0\n"
                        "    Next_Child = 0\n"
                        "    Next_Sibling = 0\n"
                        "}\n"
                        "-----------------------\n"
                        "Node  LITERAL_INT = 5:\n"
                        "    Start = test.msl/0/12\n"
                        "    First_Child = 0\n"
                        "    Next_Child = 0\n"
                        "    Next_Sibling = 0\n"
                        "}\n"
                        "-----------------------\n");
}
} // namespace
int main() {
  T t("Parser");
  t.test("literal values", test_literal);
  t.test("var def", test_var_def);
  return 0;
}
