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
IRInstr ir_new_store(LocationRef where, InternedString varname) {
  IRInstr ir;
  ir.as.VAR = varname;
  ir.where = where;
  ir.tag = IRTag::STORE;
  return ir;
}
IRInstr ir_new_load(LocationRef where, InternedString varname) {
  IRInstr ir;
  ir.as.VAR = varname;
  ir.where = where;
  ir.tag = IRTag::LOAD;
  return ir;
}
IRInstr ir_new(LocationRef where, IRTag tag) {
  IRInstr ir;
  ir.as.NOTHING = false;
  ir.where = where;
  ir.tag = tag;
  return ir;
}

void compile_one(IRContext *ctx, nodes *ns, node_idx curr_idx);
void compile_ir_var_set(IRContext *ctx, nodes *ns, node_idx curr_idx,
                        Node curr) {
  IRInstr instr = ir_new_store(curr.start, curr.as.IDENTIFIER);
  compile_one(ctx, ns, ns->first_child(curr_idx));
  ctx->add(instr);
}
void compile_ir_var_ref(IRContext *ctx, nodes *ns, Node curr) {
  IRInstr instr = ir_new_load(curr.start, curr.as.IDENTIFIER);
  ctx->add(instr);
}
void compile_ir_boolean_not(IRContext *ctx, nodes *ns, node_idx curr_idx,
                            Node curr) {
  IRInstr instr = ir_new(curr.start, IRTag::NOT);
  compile_one(ctx, ns, ns->first_child(curr_idx));
  ctx->add(instr);
}
void compile_ir_invert(IRContext *ctx, nodes *ns, node_idx curr_idx,
                       Node curr) {
  IRInstr instr = ir_new(curr.start, IRTag::INVERT);
  compile_one(ctx, ns, ns->first_child(curr_idx));
  ctx->add(instr);
}
void compile_ir_infix_add(IRContext *ctx, nodes *ns, node_idx curr_idx,
                          Node curr) {
  IRInstr instr = ir_new(curr.start, IRTag::ADD);
  compile_one(ctx, ns, ns->nth_child(curr, 0));
  compile_one(ctx, ns, ns->nth_child(curr, 1));
  ctx->add(instr);
}
void compile_ir_infix_sub(IRContext *ctx, nodes *ns, node_idx curr_idx,
                          Node curr) {
  IRInstr instr = ir_new(curr.start, IRTag::SUB);
  compile_one(ctx, ns, ns->nth_child(curr, 0));
  compile_one(ctx, ns, ns->nth_child(curr, 1));
  ctx->add(instr);
}
void compile_ir_infix_mul(IRContext *ctx, nodes *ns, node_idx curr_idx,
                          Node curr) {
  IRInstr instr = ir_new(curr.start, IRTag::MUL);
  compile_one(ctx, ns, ns->nth_child(curr, 0));
  compile_one(ctx, ns, ns->nth_child(curr, 1));
  ctx->add(instr);
}
void compile_ir_infix_div(IRContext *ctx, nodes *ns, node_idx curr_idx,
                          Node curr) {
  IRInstr instr = ir_new(curr.start, IRTag::DIV);
  compile_one(ctx, ns, ns->nth_child(curr, 0));
  compile_one(ctx, ns, ns->nth_child(curr, 1));
  ctx->add(instr);
}
void compile_ir_infix_mod(IRContext *ctx, nodes *ns, node_idx curr_idx,
                          Node curr) {
  IRInstr instr = ir_new(curr.start, IRTag::MOD);
  compile_one(ctx, ns, ns->nth_child(curr, 0));
  compile_one(ctx, ns, ns->nth_child(curr, 1));
  ctx->add(instr);
}
void compile_ir_infix_lt(IRContext *ctx, nodes *ns, node_idx curr_idx,
                         Node curr) {
  IRInstr instr = ir_new(curr.start, IRTag::LT);
  compile_one(ctx, ns, ns->nth_child(curr, 0));
  compile_one(ctx, ns, ns->nth_child(curr, 1));
  ctx->add(instr);
}
void compile_ir_infix_lte(IRContext *ctx, nodes *ns, node_idx curr_idx,
                          Node curr) {
  IRInstr instr = ir_new(curr.start, IRTag::LTE);
  compile_one(ctx, ns, ns->nth_child(curr, 0));
  compile_one(ctx, ns, ns->nth_child(curr, 1));
  ctx->add(instr);
}
void compile_ir_infix_gt(IRContext *ctx, nodes *ns, node_idx curr_idx,
                         Node curr) {
  IRInstr instr = ir_new(curr.start, IRTag::GT);
  compile_one(ctx, ns, ns->nth_child(curr, 0));
  compile_one(ctx, ns, ns->nth_child(curr, 1));
  ctx->add(instr);
}
void compile_ir_infix_gte(IRContext *ctx, nodes *ns, node_idx curr_idx,
                          Node curr) {
  IRInstr instr = ir_new(curr.start, IRTag::GTE);
  compile_one(ctx, ns, ns->nth_child(curr, 0));
  compile_one(ctx, ns, ns->nth_child(curr, 1));
  ctx->add(instr);
}
void compile_ir_infix_eq(IRContext *ctx, nodes *ns, node_idx curr_idx,
                         Node curr) {
  IRInstr instr = ir_new(curr.start, IRTag::EQ);
  compile_one(ctx, ns, ns->nth_child(curr, 0));
  compile_one(ctx, ns, ns->nth_child(curr, 1));
  ctx->add(instr);
}
void compile_ir_infix_neq(IRContext *ctx, nodes *ns, node_idx curr_idx,
                          Node curr) {
  // TODO check the ordering 0 is left, 1 is right
  // so the stack looks like that  (bottom) << leftval << rightval << (top)
  // when reading the application must be in reverse OR the writing must happen
  // in reverse. the second one might make it simpler later BUT that also means
  // that no left to right execution is guaranteed. so probably that here is
  // correct and just when reading i have to worry... think about that
  IRInstr instr = ir_new(curr.start, IRTag::NEQ);
  compile_one(ctx, ns, ns->nth_child(curr, 0));
  compile_one(ctx, ns, ns->nth_child(curr, 1));
  ctx->add(instr);
}
void compile_ir_infix_str_concat(IRContext *ctx, nodes *ns, node_idx curr_idx,
                                 Node curr) {
  IRInstr instr = ir_new(curr.start, IRTag::STR_CONCAT);
  compile_one(ctx, ns, ns->nth_child(curr, 0));
  compile_one(ctx, ns, ns->nth_child(curr, 1));
  ctx->add(instr);
}
void compile_ir_infix_and(IRContext *ctx, nodes *ns, node_idx curr_idx,
                          Node curr) {
  // TODO change that so it only evaluates right hand side if the first one is
  // true
  IRInstr instr = ir_new(curr.start, IRTag::AND);
  compile_one(ctx, ns, ns->nth_child(curr, 0));
  compile_one(ctx, ns, ns->nth_child(curr, 1));
  ctx->add(instr);
}
void compile_ir_infix_or(IRContext *ctx, nodes *ns, node_idx curr_idx,
                         Node curr) {
  // TODO change that so it only evaluates right hand side if the first one is
  // false
  IRInstr instr = ir_new(curr.start, IRTag::OR);
  compile_one(ctx, ns, ns->nth_child(curr, 0));
  compile_one(ctx, ns, ns->nth_child(curr, 1));
  ctx->add(instr);
}

void compile_one(IRContext *ctx, nodes *ns, node_idx curr_idx) {
  while (!curr_idx.is_null()) {
    Node curr = ns->at(curr_idx);
    switch (curr.tag) {
    case NodeTag::NIL:
      break;
    case NodeTag::LITERAL_INT:
      ctx->add(ir_new_push_int(curr.start, curr.as.INT));
      break;
    case NodeTag::LITERAL_FLOAT:
      ctx->add(ir_new_push_float(curr.start, curr.as.FLOAT));
      break;
    case NodeTag::LITERAL_SYMBOL:
      ctx->add(ir_new_push_symbol(curr.start, curr.as.SYMBOL));
      break;
    case NodeTag::LITERAL_STRING:
      ctx->add(ir_new_push_alloc_string(curr.start, curr.as.STRING));
      break;
    case NodeTag::VAR_DEF:
      compile_ir_var_set(ctx, ns, curr_idx, curr);
      break;
    case NodeTag::VAR_SET:
      compile_ir_var_set(ctx, ns, curr_idx, curr);
      break;
    case NodeTag::VAR_REF:
      compile_ir_var_ref(ctx, ns, curr);
      break;
    case NodeTag::PREFIX_NOT:
      compile_ir_boolean_not(ctx, ns, curr_idx, curr);
      break;
    case NodeTag::PREFIX_INVERS:
      compile_ir_invert(ctx, ns, curr_idx, curr);
      break;
    case NodeTag::INFIX_ADD:
      compile_ir_infix_add(ctx, ns, curr_idx, curr);
      break;
    case NodeTag::INFIX_SUB:
      compile_ir_infix_sub(ctx, ns, curr_idx, curr);
      break;
    case NodeTag::INFIX_MUL:
      compile_ir_infix_mul(ctx, ns, curr_idx, curr);
      break;
    case NodeTag::INFIX_DIV:
      compile_ir_infix_div(ctx, ns, curr_idx, curr);
      break;
    case NodeTag::INFIX_MOD:
      compile_ir_infix_mod(ctx, ns, curr_idx, curr);
      break;
    case NodeTag::INFIX_AND:
      compile_ir_infix_and(ctx, ns, curr_idx, curr);
      break;
    case NodeTag::INFIX_OR:
      compile_ir_infix_or(ctx, ns, curr_idx, curr);
      break;
    case NodeTag::INFIX_LT:
      compile_ir_infix_lt(ctx, ns, curr_idx, curr);
      break;
    case NodeTag::INFIX_LTE:
      compile_ir_infix_lte(ctx, ns, curr_idx, curr);
      break;
    case NodeTag::INFIX_GT:
      compile_ir_infix_gt(ctx, ns, curr_idx, curr);
      break;
    case NodeTag::INFIX_GTE:
      compile_ir_infix_gte(ctx, ns, curr_idx, curr);
      break;
    case NodeTag::INFIX_EQ:
      compile_ir_infix_eq(ctx, ns, curr_idx, curr);
      break;
    case NodeTag::INFIX_NEQ:
      compile_ir_infix_neq(ctx, ns, curr_idx, curr);
      break;
    case NodeTag::INFIX_STR_CONCAT:
      compile_ir_infix_str_concat(ctx, ns, curr_idx, curr);
      break;
    case NodeTag::FN_CALL:
    case NodeTag::FN_DEF:
    case NodeTag::IF:
    case NodeTag::RETURN:
    case NodeTag::TRY:
    case NodeTag::EXPECT:
    case NodeTag::INTERNAL_PARTIAL_CONDITION:
    case NodeTag::INTERNAL_PARTIAL_DEFAULT_CONDITION:
    case NodeTag::INTERNAL_LIST:
      break;
    }
  }
}

} // namespace

std::vector<IRInstr> compile_ir(nodes ns) {
  IRContext ctx;

  node_idx curr_idx = node_idx{ns.first_elem};
  while (!curr_idx.is_null()) {
    compile_one(&ctx, &ns, curr_idx);
    curr_idx = ns.next_sibling(curr_idx);
  }

  return ctx.instructions;
}
