#include "../msl_runtime_error.hpp"
#include "../parser/node.hpp"
#include "../vm/constants.hpp"
#include "ir_instr.hpp"
#include <vector>

namespace {
struct IRContext {
  uint64_t jmp_label_counter = 0;
  std::vector<IRInstr> instructions;
  std::vector<std::vector<IRInstr>> functions;

  uint64_t next_jmp_idx() { return jmp_label_counter++; }
  void add(const IRInstr &instr) { instructions.push_back(instr); }
  void add_function(const std::vector<IRInstr> &instr) {
    functions.push_back(instr);
  }
};

IRInstr ir_new_push_none(LocationRef where) {
  IRInstr ir;
  ir.as.NONE = true;
  ir.where = where;
  ir.tag = IRTag::PUSH_NONE;
  ir.extra.args = 0;
  return ir;
}
IRInstr ir_new_push_int(LocationRef where, int64_t val) {
  IRInstr ir;
  ir.as.INT = val;
  ir.where = where;
  ir.tag = IRTag::PUSH_INT;
  ir.extra.args = 0;
  return ir;
}
IRInstr ir_new_push_float(LocationRef where, double val) {
  IRInstr ir;
  ir.as.FLOAT = val;
  ir.where = where;
  ir.tag = IRTag::PUSH_FLOAT;
  ir.extra.args = 0;
  return ir;
}
IRInstr ir_new_push_symbol(LocationRef where, Symbol val) {
  IRInstr ir;
  ir.as.SYMBOL = val;
  ir.where = where;
  ir.tag = IRTag::PUSH_SYMBOL;
  ir.extra.args = 0;
  return ir;
}
IRInstr ir_new_push_alloc_string(LocationRef where, InternedString val) {
  IRInstr ir;
  ir.as.STRING = val;
  ir.where = where;
  ir.tag = IRTag::PUSH_ALLOC_STRING;
  ir.extra.args = 0;
  return ir;
}
IRInstr ir_new_store(LocationRef where, InternedString varname) {
  IRInstr ir;
  ir.as.VAR = varname;
  ir.where = where;
  ir.tag = IRTag::STORE;
  ir.extra.args = 0;
  return ir;
}
IRInstr ir_new_store_new(LocationRef where, InternedString varname) {
  IRInstr ir;
  ir.as.VAR = varname;
  ir.where = where;
  ir.tag = IRTag::STORE_NEW;
  ir.extra.args = 0;
  return ir;
}
IRInstr ir_new_load(LocationRef where, InternedString varname) {
  IRInstr ir;
  ir.as.VAR = varname;
  ir.where = where;
  ir.tag = IRTag::LOAD;
  ir.extra.args = 0;
  return ir;
}
IRInstr ir_new(LocationRef where, IRTag tag) {
  IRInstr ir;
  ir.as.NONE = false;
  ir.where = where;
  ir.tag = tag;
  ir.extra.args = 0;
  return ir;
}
IRInstr ir_new_call(LocationRef where, InternedString name) {
  IRInstr ir;
  ir.as.VAR = name;
  ir.where = where;
  ir.tag = IRTag::CALL;
  ir.extra.args = 0;
  return ir;
}
IRInstr ir_new_vm_call(LocationRef where, InternedString name, uint16_t args) {
  IRInstr ir;
  ir.as.VAR = name;
  ir.where = where;
  ir.tag = IRTag::VMCALL;
  ir.extra.args = args;
  return ir;
}
IRInstr ir_new_fn_init(LocationRef where, InternedString name,
                       uint16_t locals) {
  IRInstr ir;
  ir.as.VAR = name;
  ir.where = where;
  ir.tag = IRTag::FUNCTION_START;
  ir.extra.locals = locals;
  return ir;
}
IRInstr ir_new_register_arg(LocationRef where, InternedString varname) {
  IRInstr ir;
  ir.as.VAR = varname;
  ir.where = where;
  ir.tag = IRTag::REGISTER_ARG;
  ir.extra.locals = 0;
  return ir;
}
IRInstr ir_new_jump(LocationRef where, IRTag tag, Label target) {
  IRInstr ir;
  ir.as.LABEL = target;
  ir.where = where;
  ir.tag = tag;
  ir.extra.args = 0;
  return ir;
}
IRInstr ir_new_label(LocationRef where, Label label) {
  IRInstr ir;
  ir.as.LABEL = label;
  ir.where = where;
  ir.tag = IRTag::LABEL;
  ir.extra.args = 0;
  return ir;
}

void compile_one(IRContext *ctx, nodes *ns, node_idx curr_idx,
                 bool is_standalone);
void compile_ir_var_set(IRContext *ctx, nodes *ns, node_idx curr_idx,
                        Node curr) {
  IRInstr instr = ir_new_store(curr.start, curr.as.IDENTIFIER);
  compile_one(ctx, ns, ns->first_child(curr_idx), false);
  ctx->add(instr);
}
void compile_ir_var_def(IRContext *ctx, nodes *ns, node_idx curr_idx,
                        Node curr) {
  IRInstr instr = ir_new_store_new(curr.start, curr.as.IDENTIFIER);
  compile_one(ctx, ns, ns->first_child(curr_idx), false);
  ctx->add(instr);
}
void compile_ir_var_ref(IRContext *ctx, Node curr) {
  IRInstr instr = ir_new_load(curr.start, curr.as.IDENTIFIER);
  ctx->add(instr);
}
void compile_ir_boolean_not(IRContext *ctx, nodes *ns, node_idx curr_idx,
                            Node curr) {
  IRInstr instr = ir_new(curr.start, IRTag::NOT);
  compile_one(ctx, ns, ns->first_child(curr_idx), false);
  ctx->add(instr);
}
void compile_ir_invert(IRContext *ctx, nodes *ns, node_idx curr_idx,
                       Node curr) {
  IRInstr instr = ir_new(curr.start, IRTag::INVERT);
  compile_one(ctx, ns, ns->first_child(curr_idx), false);
  ctx->add(instr);
}
void compile_ir_infix_add(IRContext *ctx, nodes *ns, Node curr) {
  IRInstr instr = ir_new(curr.start, IRTag::ADD);
  compile_one(ctx, ns, ns->nth_child(curr, 0), false);
  compile_one(ctx, ns, ns->nth_child(curr, 1), false);
  ctx->add(instr);
}
void compile_ir_infix_sub(IRContext *ctx, nodes *ns, Node curr) {
  IRInstr instr = ir_new(curr.start, IRTag::SUB);
  compile_one(ctx, ns, ns->nth_child(curr, 0), false);
  compile_one(ctx, ns, ns->nth_child(curr, 1), false);
  ctx->add(instr);
}
void compile_ir_infix_mul(IRContext *ctx, nodes *ns, Node curr) {
  IRInstr instr = ir_new(curr.start, IRTag::MUL);
  compile_one(ctx, ns, ns->nth_child(curr, 0), false);
  compile_one(ctx, ns, ns->nth_child(curr, 1), false);
  ctx->add(instr);
}
void compile_ir_infix_div(IRContext *ctx, nodes *ns, Node curr) {
  IRInstr instr = ir_new(curr.start, IRTag::DIV);
  compile_one(ctx, ns, ns->nth_child(curr, 0), false);
  compile_one(ctx, ns, ns->nth_child(curr, 1), false);
  ctx->add(instr);
}
void compile_ir_infix_mod(IRContext *ctx, nodes *ns, Node curr) {
  IRInstr instr = ir_new(curr.start, IRTag::MOD);
  compile_one(ctx, ns, ns->nth_child(curr, 0), false);
  compile_one(ctx, ns, ns->nth_child(curr, 1), false);
  ctx->add(instr);
}
void compile_ir_infix_lt(IRContext *ctx, nodes *ns, Node curr) {
  IRInstr instr = ir_new(curr.start, IRTag::LT);
  compile_one(ctx, ns, ns->nth_child(curr, 0), false);
  compile_one(ctx, ns, ns->nth_child(curr, 1), false);
  ctx->add(instr);
}
void compile_ir_infix_lte(IRContext *ctx, nodes *ns, Node curr) {
  IRInstr instr = ir_new(curr.start, IRTag::LTE);
  compile_one(ctx, ns, ns->nth_child(curr, 0), false);
  compile_one(ctx, ns, ns->nth_child(curr, 1), false);
  ctx->add(instr);
}
void compile_ir_infix_gt(IRContext *ctx, nodes *ns, Node curr) {
  IRInstr instr = ir_new(curr.start, IRTag::GT);
  compile_one(ctx, ns, ns->nth_child(curr, 0), false);
  compile_one(ctx, ns, ns->nth_child(curr, 1), false);
  ctx->add(instr);
}
void compile_ir_infix_gte(IRContext *ctx, nodes *ns, Node curr) {
  IRInstr instr = ir_new(curr.start, IRTag::GTE);
  compile_one(ctx, ns, ns->nth_child(curr, 0), false);
  compile_one(ctx, ns, ns->nth_child(curr, 1), false);
  ctx->add(instr);
}
void compile_ir_infix_eq(IRContext *ctx, nodes *ns, Node curr) {
  IRInstr instr = ir_new(curr.start, IRTag::EQ);
  compile_one(ctx, ns, ns->nth_child(curr, 0), false);
  compile_one(ctx, ns, ns->nth_child(curr, 1), false);
  ctx->add(instr);
}
void compile_ir_infix_neq(IRContext *ctx, nodes *ns, Node curr) {
  IRInstr instr = ir_new(curr.start, IRTag::NEQ);
  compile_one(ctx, ns, ns->nth_child(curr, 0), false);
  compile_one(ctx, ns, ns->nth_child(curr, 1), false);
  ctx->add(instr);
}
void compile_ir_infix_str_concat(IRContext *ctx, nodes *ns, Node curr) {
  IRInstr instr =
      ir_new_vm_call(curr.start, Constants::BUILDIN_FN_STR_CONCAT, 2);
  compile_one(ctx, ns, ns->nth_child(curr, 0), false);
  compile_one(ctx, ns, ns->nth_child(curr, 1), false);
  ctx->add(instr);
}
void compile_ir_return(IRContext *ctx, nodes *ns, Node curr) {
  compile_one(ctx, ns, ns->nth_child(curr, 0), false);
  IRInstr instr = ir_new(curr.start, IRTag::RETURN);
  ctx->add(instr);
}
void compile_ir_try(IRContext *ctx, nodes *ns, Node curr) {
  compile_one(ctx, ns, ns->nth_child(curr, 0), false);
  IRInstr dup = ir_new(curr.start, IRTag::DUP);
  ctx->add(dup);
  IRInstr typeof_instr = ir_new(curr.start, IRTag::TYPEOF);
  ctx->add(typeof_instr);
  IRInstr compare_value_push =
      ir_new_push_symbol(curr.start, create_symbol("#error"));
  ctx->add(compare_value_push);
  IRInstr compare_result = ir_new(curr.start, IRTag::EQ);
  ctx->add(compare_result);
  Label label_skip_return = create_next_label("TRY");
  IRInstr skip_return =
      ir_new_jump(curr.start, IRTag::JMPIFN, label_skip_return);
  ctx->add(skip_return);
  IRInstr instr = ir_new(curr.start, IRTag::RETURN);
  ctx->add(instr);
  IRInstr label_skip_return_instr = ir_new_label(curr.start, label_skip_return);
  ctx->add(label_skip_return_instr);
}
void compile_ir_expect(IRContext *ctx, nodes *ns, Node curr) {
  compile_one(ctx, ns, ns->nth_child(curr, 0), false);
  IRInstr dup = ir_new(curr.start, IRTag::DUP);
  ctx->add(dup);
  IRInstr typeof_instr = ir_new(curr.start, IRTag::TYPEOF);
  ctx->add(typeof_instr);
  IRInstr compare_value_push =
      ir_new_push_symbol(curr.start, Constants::SYM_T_ERROR);
  ctx->add(compare_value_push);
  IRInstr compare_result = ir_new(curr.start, IRTag::EQ);
  ctx->add(compare_result);
  Label label_skip_error = create_next_label("EXPECT");
  IRInstr skip_panic = ir_new_jump(curr.start, IRTag::JMPIFN, label_skip_error);
  ctx->add(skip_panic);
  IRInstr push_panic_msg =
      ir_new_push_alloc_string(curr.start, Constants::EXPECT_ERROR_MSG);
  ctx->add(push_panic_msg);
  IRInstr instr = ir_new_vm_call(curr.start, Constants::BUILDIN_FN_PANIC, 1);
  ctx->add(instr);
  IRInstr label_skip_return_instr = ir_new_label(curr.start, label_skip_error);
  ctx->add(label_skip_return_instr);
}

void compile_ir_fn_call(IRContext *ctx, nodes *ns, Node curr) {
  // compile args left to right
  uint64_t i = 1;
  node_idx arg_idx = ns->nth_child(curr, i);
  while (!arg_idx.is_null()) {
    compile_one(ctx, ns, arg_idx, false);
    i++;
    arg_idx = ns->nth_child(curr, i);
  }
  uint16_t args_count = i - 1;

  // call
  InternedString fn_name = ns->at(ns->nth_child(curr, 0)).as.IDENTIFIER;
  bool is_vm_fn = fn_name.index == Constants::BUILDIN_FN_PANIC.index ||
                  fn_name.index == Constants::BUILDIN_FN_PUT.index ||
                  fn_name.index == Constants::BUILDIN_FN_AT.index ||
                  fn_name.index == Constants::BUILDIN_FN_LIST.index ||
                  fn_name.index == Constants::BUILDIN_FN_CONCAT.index ||
                  fn_name.index == Constants::BUILDIN_FN_APPEND.index ||
                  fn_name.index == Constants::BUILDIN_FN_PRINT.index ||
                  fn_name.index == Constants::BUILDIN_FN_PREPEND.index;
  IRInstr call_instr;
  if (is_vm_fn) {
    call_instr = ir_new_vm_call(curr.start, fn_name, args_count);
  } else {
    call_instr = ir_new_call(curr.start, fn_name);
  }
  ctx->add(call_instr);
}

void compile_ir_fn_def(IRContext *ctx, nodes *ns, Node curr) {

  IRContext fn_ctx = IRContext();

  InternedString fn_name = curr.as.IDENTIFIER;
  // SET LOCALS COUNT TO 0
  IRInstr frame_init = ir_new_fn_init(curr.start, fn_name, 0);
  fn_ctx.add(frame_init);
  // SAVE POSITION FOR LATER PATCHING OF LOCALS COUNT
  uint64_t frame_init_relative_position = fn_ctx.instructions.size() - 1;

  node_idx args_list_head = ns->nth_child(curr, 0);
  uint64_t args_i = 0;
  node_idx arg_idx = ns->nth_child(args_list_head, args_i);
  while (!arg_idx.is_null()) {
    Node arg = ns->at(arg_idx);
    fn_ctx.add(ir_new_register_arg(arg.start, arg.as.IDENTIFIER));
    ++args_i;
    arg_idx = ns->nth_child(args_list_head, args_i);
  }

  uint64_t i = 1;
  node_idx body_idx = ns->nth_child(curr, 1);
  while (!body_idx.is_null()) {
    compile_one(&fn_ctx, ns, body_idx, true);
    ++i;
    body_idx = ns->nth_child(curr, i);
  }
  IRInstr push_default = ir_new_push_none(curr.start);
  fn_ctx.add(push_default);
  IRInstr default_return = ir_new(curr.start, IRTag::RETURN);
  fn_ctx.add(default_return);

  // PATCH LOCALS COUNT
  uint16_t VARS = 0;
  for (auto instr : fn_ctx.instructions) {
    if (instr.tag == IRTag::STORE_NEW) {
      ++VARS;
    }
  }
  fn_ctx.instructions.at(frame_init_relative_position).extra.locals = VARS;
  fn_ctx.add(ir_new(curr.start, IRTag::FUNCTION_END));
  ctx->add_function(fn_ctx.instructions);
}
void compile_ir_infix_and(IRContext *ctx, nodes *ns, Node curr) {
  Label label_end = create_next_label("AND_END");
  compile_one(ctx, ns, ns->nth_child(curr, 0), false);
  ctx->add(ir_new_jump(curr.start, IRTag::PEEK_JMPIFN, label_end));
  ctx->add(ir_new(curr.start, IRTag::POP));
  compile_one(ctx, ns, ns->nth_child(curr, 1), false);
  ctx->add(ir_new_label(curr.start, label_end));
}
void compile_ir_infix_or(IRContext *ctx, nodes *ns, Node curr) {
  Label label_end = create_next_label("OR_END");
  compile_one(ctx, ns, ns->nth_child(curr, 0), false);
  ctx->add(ir_new_jump(curr.start, IRTag::PEEK_JMPIF, label_end));
  ctx->add(ir_new(curr.start, IRTag::POP));
  compile_one(ctx, ns, ns->nth_child(curr, 1), false);
  ctx->add(ir_new_label(curr.start, label_end));
}

void compile_condition_body(IRContext *ctx, nodes *ns, node_idx body_head) {
  ctx->add(ir_new(ns->at(body_head).start, IRTag::SCOPE_START));
  node_idx curr = body_head;
  while (!curr.is_null()) {
    compile_one(ctx, ns, curr, true);
    curr = ns->next_child(curr);
  }
  ctx->add(ir_new(ns->at(body_head).start, IRTag::SCOPE_END));
}
void compile_ir_if(IRContext *ctx, nodes *ns, node_idx curr_idx, Node curr) {
  Label label_end = create_next_label("CONDITIONAL_END");
  std::vector<Label> skip_labels;
  for (uint64_t i = 0; i < ns->list_length(ns->nth_child(curr_idx, 0)); i++) {
    skip_labels.push_back(create_next_label("SKIP_LABEL_" + std::to_string(i)));
  }

  node_idx if_cond = ns->nth_child(curr_idx, 0);
  node_idx if_cond_condition = ns->nth_child(if_cond, 0);
  compile_one(ctx, ns, if_cond_condition, false);
  if (skip_labels.size() > 0) {
    IRInstr ifjmp = ir_new_jump(curr.start, IRTag::JMPIFN, skip_labels[0]);
    ctx->add(ifjmp);
  } else {
    IRInstr ifjmp = ir_new_jump(curr.start, IRTag::JMPIFN, label_end);
    ctx->add(ifjmp);
  }
  compile_condition_body(ctx, ns, ns->nth_child(if_cond, 1));

  uint32_t cond_idx = 1;
  node_idx elif_cond = ns->nth_child(curr_idx, cond_idx);
  while (!elif_cond.is_null() &&
         ns->at(elif_cond).tag != NodeTag::INTERNAL_PARTIAL_DEFAULT_CONDITION) {
    // skip rest for previous hit
    ctx->add(ir_new_jump(curr.start, IRTag::JMP, label_end));
    // insert skip label for previous condition
    IRInstr elif_skip_label =
        ir_new_jump(curr.start, IRTag::LABEL, skip_labels.at(cond_idx - 1));
    ctx->add(elif_skip_label);

    // actual condition
    node_idx elif_cond_condition = ns->nth_child(elif_cond, 0);
    compile_one(ctx, ns, elif_cond_condition, false);

    // jmp to next/end if false
    IRInstr elifjmp;
    if (skip_labels.size() > cond_idx) {
      elifjmp =
          ir_new_jump(curr.start, IRTag::JMPIFN, skip_labels.at(cond_idx));
    } else {
      elifjmp = ir_new_jump(curr.start, IRTag::JMPIFN, label_end);
    }
    ctx->add(elifjmp);

    // actual body
    compile_condition_body(ctx, ns, ns->nth_child(elif_cond, 1));
    cond_idx++;
    elif_cond = ns->nth_child(curr_idx, cond_idx);
  }

  if (!elif_cond.is_null() &&
      ns->at(elif_cond).tag == NodeTag::INTERNAL_PARTIAL_DEFAULT_CONDITION) {
    // skip rest for previous hit
    ctx->add(ir_new_jump(curr.start, IRTag::JMP, label_end));
    // insert skip label for previous condition
    IRInstr else_skip_label =
        ir_new_jump(curr.start, IRTag::LABEL, skip_labels.back());
    ctx->add(else_skip_label);

    // actual body
    compile_condition_body(ctx, ns, ns->nth_child(elif_cond, 0));
  }

  ctx->add(ir_new_label(curr.start, label_end));
}

void compile_one(IRContext *ctx, nodes *ns, node_idx curr_idx,
                 bool is_standalone) {
  if (curr_idx.is_null()) {
    throw std::runtime_error("attempt to compile NIL node");
  } else {
    Node curr = ns->at(curr_idx);
    switch (curr.tag) {
    case NodeTag::NIL:
      break;
    case NodeTag::LITERAL_INT:
      if (!is_standalone) {
        ctx->add(ir_new_push_int(curr.start, curr.as.INT));
      }
      break;
    case NodeTag::LITERAL_FLOAT:
      if (!is_standalone) {
        ctx->add(ir_new_push_float(curr.start, curr.as.FLOAT));
      }
      break;
    case NodeTag::LITERAL_SYMBOL:
      if (!is_standalone) {
        ctx->add(ir_new_push_symbol(curr.start, curr.as.SYMBOL));
      }
      break;
    case NodeTag::LITERAL_STRING:
      if (!is_standalone) {
        ctx->add(ir_new_push_alloc_string(curr.start, curr.as.STRING));
      }
      break;
    case NodeTag::VAR_DEF:
      compile_ir_var_def(ctx, ns, curr_idx, curr);
      break;
    case NodeTag::VAR_SET:
      compile_ir_var_set(ctx, ns, curr_idx, curr);
      break;
    case NodeTag::VAR_REF:
      if (!is_standalone) {
        compile_ir_var_ref(ctx, curr);
      }
      break;
    case NodeTag::PREFIX_NOT:
      compile_ir_boolean_not(ctx, ns, curr_idx, curr);
      if (is_standalone) {
        ctx->add(ir_new(curr.start, IRTag::POP));
      }
      break;
    case NodeTag::PREFIX_INVERS:
      compile_ir_invert(ctx, ns, curr_idx, curr);
      if (is_standalone) {
        ctx->add(ir_new(curr.start, IRTag::POP));
      }
      break;
    case NodeTag::INFIX_ADD:
      compile_ir_infix_add(ctx, ns, curr);
      if (is_standalone) {
        ctx->add(ir_new(curr.start, IRTag::POP));
      }
      break;
    case NodeTag::INFIX_SUB:
      compile_ir_infix_sub(ctx, ns, curr);
      if (is_standalone) {
        ctx->add(ir_new(curr.start, IRTag::POP));
      }
      break;
    case NodeTag::INFIX_MUL:
      compile_ir_infix_mul(ctx, ns, curr);
      if (is_standalone) {
        ctx->add(ir_new(curr.start, IRTag::POP));
      }
      break;
    case NodeTag::INFIX_DIV:
      compile_ir_infix_div(ctx, ns, curr);
      if (is_standalone) {
        ctx->add(ir_new(curr.start, IRTag::POP));
      }
      break;
    case NodeTag::INFIX_MOD:
      compile_ir_infix_mod(ctx, ns, curr);
      if (is_standalone) {
        ctx->add(ir_new(curr.start, IRTag::POP));
      }
      break;
    case NodeTag::INFIX_AND:
      compile_ir_infix_and(ctx, ns, curr);
      if (is_standalone) {
        ctx->add(ir_new(curr.start, IRTag::POP));
      }
      break;
    case NodeTag::INFIX_OR:
      compile_ir_infix_or(ctx, ns, curr);
      if (is_standalone) {
        ctx->add(ir_new(curr.start, IRTag::POP));
      }
      break;
    case NodeTag::INFIX_LT:
      compile_ir_infix_lt(ctx, ns, curr);
      if (is_standalone) {
        ctx->add(ir_new(curr.start, IRTag::POP));
      }
      break;
    case NodeTag::INFIX_LTE:
      compile_ir_infix_lte(ctx, ns, curr);
      if (is_standalone) {
        ctx->add(ir_new(curr.start, IRTag::POP));
      }
      break;
    case NodeTag::INFIX_GT:
      compile_ir_infix_gt(ctx, ns, curr);
      if (is_standalone) {
        ctx->add(ir_new(curr.start, IRTag::POP));
      }
      break;
    case NodeTag::INFIX_GTE:
      compile_ir_infix_gte(ctx, ns, curr);
      if (is_standalone) {
        ctx->add(ir_new(curr.start, IRTag::POP));
      }
      break;
    case NodeTag::INFIX_EQ:
      compile_ir_infix_eq(ctx, ns, curr);
      if (is_standalone) {
        ctx->add(ir_new(curr.start, IRTag::POP));
      }
      break;
    case NodeTag::INFIX_NEQ:
      compile_ir_infix_neq(ctx, ns, curr);
      if (is_standalone) {
        ctx->add(ir_new(curr.start, IRTag::POP));
      }
      break;
    case NodeTag::INFIX_STR_CONCAT:
      compile_ir_infix_str_concat(ctx, ns, curr);
      if (is_standalone) {
        ctx->add(ir_new(curr.start, IRTag::POP));
      }
      break;
    case NodeTag::FN_CALL:
      compile_ir_fn_call(ctx, ns, curr);
      if (is_standalone) {
        ctx->add(ir_new(curr.start, IRTag::POP));
      }
      break;
    case NodeTag::FN_DEF:
      compile_ir_fn_def(ctx, ns, curr);
      break;
    case NodeTag::RETURN:
      compile_ir_return(ctx, ns, curr);
      break;
    case NodeTag::TRY:
      compile_ir_try(ctx, ns, curr);
      if (is_standalone) {
        ctx->add(ir_new(curr.start, IRTag::POP));
      }
      break;
    case NodeTag::EXPECT:
      compile_ir_expect(ctx, ns, curr);
      if (is_standalone) {
        ctx->add(ir_new(curr.start, IRTag::POP));
      }
      break;
    case NodeTag::IF:
      compile_ir_if(ctx, ns, curr_idx, curr);
      break;
    case NodeTag::INTERNAL_PARTIAL_CONDITION:
      throw msl_runtime_error(curr.start,
                              "unexpected partial condition encountered");
    case NodeTag::INTERNAL_PARTIAL_DEFAULT_CONDITION:
      throw msl_runtime_error(curr.start,
                              "unexpected partial condition encountered");
    case NodeTag::INTERNAL_LIST:
      throw msl_runtime_error(curr.start,
                              "unexpected internal list encountered");
      break;
    }
  }
}

} // namespace

std::vector<IRInstr> compile_ir(nodes ns) {
  IRContext ctx;

  node_idx curr_idx = node_idx{ns.first_elem};
  while (!curr_idx.is_null()) {
    compile_one(&ctx, &ns, curr_idx, true);
    curr_idx = ns.next_sibling(curr_idx);
  }
  // stop program execution before reaching functions
  if (ctx.functions.size() != 0) {
    ctx.add(ir_new({0}, IRTag::HALT));
  }
  // append all functions to main instruction vector
  for (auto fn : ctx.functions) {
    ctx.instructions.insert(ctx.instructions.end(), fn.begin(), fn.end());
  }

  return ctx.instructions;
}
