#include "../parser/node.hpp"
#include "ir_instr.hpp"
#include <vector>

namespace {
struct IRContext {
  uint64_t jmp_label_counter = 0;
  std::vector<IRInstr> instructions;

  uint64_t next_jmp_idx() { return jmp_label_counter++; }
  void add(const IRInstr &instr) { instructions.push_back(instr); }
};

IRInstr ir_new_push_int(LocationRef where, int64_t val) {
  IRInstr ir;
  ir.as.INT = val;
  ir.where = where;
  ir.tag = IRTag::PUSH_INT;
  return ir;
}
IRInstr ir_new_push_float(LocationRef where, double val) {
  IRInstr ir;
  ir.as.FLOAT = val;
  ir.where = where;
  ir.tag = IRTag::PUSH_FLOAT;
  return ir;
}
IRInstr ir_new_push_symbol(LocationRef where, Symbol val) {
  IRInstr ir;
  ir.as.SYMBOL = val;
  ir.where = where;
  ir.tag = IRTag::PUSH_SYMBOL;
  return ir;
}
IRInstr ir_new_push_alloc_string(LocationRef where, InternedString val) {
  IRInstr ir;
  ir.as.STRING = val;
  ir.where = where;
  ir.tag = IRTag::PUSH_ALLOC_STRING;
  return ir;
}

void compile_ir_var_def(IRContext *ctx, Node curr) {
  // TODO do stuff here
}
} // namespace

std::vector<IRInstr> compile_ir(nodes ns) {
  IRContext ctx;

  node_idx curr_idx = node_idx{ns.first_elem};
  while (!curr_idx.is_null()) {
    Node curr = ns.at(curr_idx);
    switch (curr.tag) {
    case NodeTag::NIL:
      break;
    case NodeTag::LITERAL_INT:
      ctx.add(ir_new_push_int(curr.start, curr.as.INT));
      break;
    case NodeTag::LITERAL_FLOAT:
      ctx.add(ir_new_push_float(curr.start, curr.as.FLOAT));
      break;
    case NodeTag::LITERAL_SYMBOL:
      ctx.add(ir_new_push_symbol(curr.start, curr.as.SYMBOL));
      break;
    case NodeTag::LITERAL_STRING:
      ctx.add(ir_new_push_alloc_string(curr.start, curr.as.STRING));
      break;
    case NodeTag::VAR_DEF:
      compile_ir_var_def(&ctx, curr);
      break;
    case NodeTag::VAR_SET:
    case NodeTag::VAR_REF:
    case NodeTag::FN_CALL:
    case NodeTag::FN_DEF:
    case NodeTag::PREFIX_NOT:
    case NodeTag::PREFIX_INVERS:
    case NodeTag::INFIX_ADD:
    case NodeTag::INFIX_SUB:
    case NodeTag::INFIX_MUL:
    case NodeTag::INFIX_DIV:
    case NodeTag::INFIX_MOD:
    case NodeTag::INFIX_AND:
    case NodeTag::INFIX_OR:
    case NodeTag::INFIX_LT:
    case NodeTag::INFIX_LTE:
    case NodeTag::INFIX_GT:
    case NodeTag::INFIX_GTE:
    case NodeTag::INFIX_EQ:
    case NodeTag::INFIX_NEQ:
    case NodeTag::INFIX_STR_CONCAT:
    case NodeTag::IF:
    case NodeTag::RETURN:
    case NodeTag::TRY:
    case NodeTag::EXPECT:
    case NodeTag::INTERNAL_PARTIAL_CONDITION:
    case NodeTag::INTERNAL_PARTIAL_DEFAULT_CONDITION:
    case NodeTag::INTERNAL_LIST:
      break;
    }
    curr_idx = ns.next_sibling(curr_idx);
  }
  return ctx.instructions;
}
