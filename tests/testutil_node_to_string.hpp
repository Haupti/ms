#pragma once
#include "../src/interned_string.hpp"
#include "../src/parser/node.hpp"
#include <string>

inline std::string show_node(const Node &node) {
  std::string s = "Node ";
  switch (node.tag) {
  case NodeTag::NIL:
    s += " NIL";
    break;
  case NodeTag::FN_CALL:
    s += " FN_CALL";
    break;
  case NodeTag::FN_DEF:
    s += " FN_DEF = " + resolve_interned_string(node.as.IDENTIFIER);
    break;
  case NodeTag::VAR_DEF:
    s += " VAR_DEF = " + resolve_interned_string(node.as.IDENTIFIER);
    break;
  case NodeTag::VAR_SET:
    s += " VAR_SET = " + resolve_interned_string(node.as.IDENTIFIER);
    break;
  case NodeTag::VAR_REF:
    s += " VAR_REF = " + resolve_interned_string(node.as.IDENTIFIER);
    break;
  case NodeTag::LITERAL_INT:
    s += " LITERAL_INT = " + std::to_string(node.as.INT);
    break;
  case NodeTag::LITERAL_STRING:
    s += " LITERAL_STRING = '" + resolve_interned_string(node.as.STRING) + "'";
    break;
  case NodeTag::LITERAL_FLOAT:
    s += " LITERAL_FLOAT = " + std::to_string(node.as.FLOAT);
    break;
  case NodeTag::LITERAL_SYMBOL:
    s += " LITERAL_SYMBOL = " + resolve_symbol(node.as.SYMBOL);
    break;
  case NodeTag::PREFIX_NOT:
    s += " NOT";
    break;
  case NodeTag::PREFIX_INVERS:
    s += " PREFIX(-)";
    break;
  case NodeTag::IF:
    s += " IF";
    break;
  case NodeTag::INTERNAL_PARTIAL_CONDITION:
    s += " PARTIAL_CONDITION";
    break;
  case NodeTag::INTERNAL_PARTIAL_DEFAULT_CONDITION:
    s += " PARTIAL_DEFAULT_CONDITION";
    break;
  case NodeTag::FUNCTION:
    s += " FUNCTION = " + resolve_interned_string(node.as.IDENTIFIER);
    break;
  case NodeTag::INTERNAL_LIST:
    s += " LIST";
    break;
  case NodeTag::INFIX_ADD:
    s += " INFIX(+)";
    break;
  case NodeTag::INFIX_SUB:
    s += " INFIX(-)";
    break;
  case NodeTag::INFIX_MUL:
    s += " INFIX(*)";
    break;
  case NodeTag::INFIX_DIV:
    s += " INFIX(/)";
    break;
  case NodeTag::INFIX_MOD:
    s += " INFIX(mod)";
    break;
  case NodeTag::INFIX_AND:
    s += " INFIX(and)";
    break;
  case NodeTag::INFIX_OR:
    s += " INFIX(or)";
    break;
  case NodeTag::INFIX_LT:
    s += " INFIX(<)";
    break;
  case NodeTag::INFIX_LTE:
    s += " INFIX(<=)";
    break;
  case NodeTag::INFIX_GT:
    s += " INFIX(>)";
    break;
  case NodeTag::INFIX_GTE:
    s += " INFIX(>=)";
    break;
  case NodeTag::INFIX_EQ:
    s += " INFIX(==)";
    break;
  case NodeTag::INFIX_NEQ:
    s += " INFIX(!=)";
    break;
  case NodeTag::INFIX_STR_CONCAT:
    s += " INFIX(<>)";
    break;
  case NodeTag::RETURN:
    s += " RETURN";
    break;
  }
  s += ":\n";
  if (node.tag != NodeTag::NIL) {
    Location loc = resolve_location(node.start);
    s += "    Start = " + std::string(loc.filename) + "/" +
         std::to_string(loc.row) + "/" + std::to_string(loc.col) + "\n";
  } else {
    s += "    Start = NIL\n";
  }

  s += "    First_Child = " + std::to_string(node.first_child.idx) + "\n";
  s += "    Next_Child = " + std::to_string(node.next_child.idx) + "\n";
  s += "    Next_Sibling = " + std::to_string(node.next_sibling.idx) + "\n";

  return s;
}
