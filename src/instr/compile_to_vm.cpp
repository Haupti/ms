#include "../ir/ir_instr.hpp"
#include "../vm/vm_instr.hpp"
#include <cassert>
#include <unordered_map>
#include <vector>

// this has the following assumptions for the IR:
// * all variable names are unique per frame
// * all labels are unique
// * all functions are placed at the end of the IR instruction vector with
//    nothing in between
namespace {
VMInstr build(LocationRef where, VMTag tag) {
  VMInstr o;
  o.where = where;
  o.as.NONE = false;
  o.tag = tag;
  o.extra.args = 0;
  return o;
}
VMInstr build_int(LocationRef where, VMTag tag, int64_t value) {
  VMInstr o;
  o.where = where;
  o.as.INT = value;
  o.tag = tag;
  o.extra.args = 0;
  return o;
}
VMInstr build_float(LocationRef where, VMTag tag, double value) {
  VMInstr o;
  o.where = where;
  o.as.FLOAT = value;
  o.tag = tag;
  o.extra.args = 0;
  return o;
}
VMInstr build_string(LocationRef where, VMTag tag, InternedString value) {
  VMInstr o;
  o.where = where;
  o.as.STRING = value;
  o.tag = tag;
  o.extra.args = 0;
  return o;
}
VMInstr build_symbol(LocationRef where, VMTag tag, Symbol value) {
  VMInstr o;
  o.where = where;
  o.as.SYMBOL = value;
  o.tag = tag;
  o.extra.args = 0;
  return o;
}
VMInstr build_addr_acc(LocationRef where, VMTag tag, StkAddr value) {
  VMInstr o;
  o.where = where;
  o.as.STKADDR = value;
  o.tag = tag;
  o.extra.args = 0;
  return o;
}
VMInstr build_none(LocationRef where) {
  VMInstr o;
  o.where = where;
  o.as.NONE = true;
  o.tag = VMTag::PUSH_NONE;
  o.extra.args = 0;
  return o;
}
VMInstr build_vmcall(LocationRef where, uint16_t args) {
  VMInstr o;
  o.where = where;
  o.as.NONE = false;
  o.tag = VMTag::VMCALL;
  o.extra.args = args;
  return o;
}
VMInstr build_init_frame(LocationRef where, uint16_t locals) {
  VMInstr o;
  o.where = where;
  o.as.NONE = false;
  o.tag = VMTag::INIT_FRAME;
  o.extra.locals = locals;
  return o;
}
VMInstr build_jump(LocationRef where, VMTag tag, InstrAddr addr) {
  VMInstr o;
  o.where = where;
  o.as.INSTRADDR = addr;
  o.tag = tag;
  o.extra.args = 0;
  return o;
}

} // namespace

std::vector<VMInstr> compile_to_vm(std::vector<IRInstr> ir) {
  std::vector<VMInstr> instructions;
  instructions.reserve(ir.size());

  // uint64_t of interned string
  std::unordered_map<uint64_t, StkAddr> global_var_map;
  // uint64_t of interned string
  std::unordered_map<uint64_t, StkAddr> local_var_map;
  // uint64_t of interned string, maps to functions and labels
  std::unordered_map<uint64_t, InstrAddr> label_map;
  StkAddr global_stkvars_ptr;
  StkAddr local_stkvars_ptr;
  InstrAddr current_instr_ptr;

  // TODO
  // new plan for finding local/global load/store, and resolving which variable to choose
  // make a vector of depth marked variables (depth + varname + stack address)
  // remember depth
  // on store_new push one back at current depth
  // on load search from back of that array, first matching is the one
  // on scope_start/function_start increase depth
  // on scope_end/function_end decrease depth
  // on depth 0 and load/store its a global load/store -> use 0 offset
  // otherwise its a local one -> use the current offset

  for (auto instr : ir) {
    LocationRef where = instr.where;
    switch (instr.tag) {
    case IRTag::PUSH_INT: {
      auto o = build_int(where, VMTag::PUSH_INT, instr.as.INT);
      instructions.push_back(o);
      ++current_instr_ptr.addr;
    } break;
    case IRTag::PUSH_FLOAT: {
      auto o = build_float(where, VMTag::PUSH_FLOAT, instr.as.FLOAT);
      instructions.push_back(o);
      ++current_instr_ptr.addr;
    } break;
    case IRTag::PUSH_ALLOC_STRING: {
      auto o = build_string(where, VMTag::PUSH_ALLOC_STRING, instr.as.STRING);
      instructions.push_back(o);
      ++current_instr_ptr.addr;
    } break;
    case IRTag::PUSH_SYMBOL: {
      VMInstr o = build_symbol(where, VMTag::PUSH_SYMBOL, instr.as.SYMBOL);
      instructions.push_back(o);
      ++current_instr_ptr.addr;
    } break;
    case IRTag::PUSH_NONE: {
      VMInstr o = build_none(where);
      instructions.push_back(o);
      ++current_instr_ptr.addr;
    } break;
    case IRTag::STORE: {
      StkAddr addr = local_var_map.at(instr.as.VAR.index);
      VMInstr o = build_addr_acc(where, VMTag::STORE, addr);
      instructions.push_back(o);
      ++current_instr_ptr.addr;
    } break;
    case IRTag::STORE_NEW: {
      local_var_map.at(instr.as.VAR.index) = local_stkvars_ptr;
      local_stkvars_ptr.addr += 1;

      StkAddr addr = local_var_map.at(instr.as.VAR.index);
      VMInstr o = build_addr_acc(where, VMTag::STORE, addr);
      instructions.push_back(o);
      ++current_instr_ptr.addr;
    } break;
    case IRTag::LOAD: {
      StkAddr addr = local_var_map.at(instr.as.VAR.index);
      VMInstr o = build_addr_acc(where, VMTag::LOAD, addr);
      instructions.push_back(o);
      ++current_instr_ptr.addr;
    } break;
    case IRTag::NOT: {
      VMInstr o = build(where, VMTag::NOT);
      instructions.push_back(o);
      ++current_instr_ptr.addr;
    } break;
    case IRTag::INVERT: {
      VMInstr o = build(where, VMTag::INVERT);
      instructions.push_back(o);
      ++current_instr_ptr.addr;
    } break;
    case IRTag::ADD: {
      VMInstr o = build(where, VMTag::ADD);
      instructions.push_back(o);
      ++current_instr_ptr.addr;
    } break;
    case IRTag::SUB: {
      VMInstr o = build(where, VMTag::SUB);
      instructions.push_back(o);
      ++current_instr_ptr.addr;
    } break;
    case IRTag::MUL: {
      VMInstr o = build(where, VMTag::MUL);
      instructions.push_back(o);
      ++current_instr_ptr.addr;
    } break;
    case IRTag::DIV: {
      VMInstr o = build(where, VMTag::DIV);
      instructions.push_back(o);
      ++current_instr_ptr.addr;
    } break;
    case IRTag::MOD: {
      VMInstr o = build(where, VMTag::MOD);
      instructions.push_back(o);
      ++current_instr_ptr.addr;
    } break;
    case IRTag::LT: {
      VMInstr o = build(where, VMTag::LT);
      instructions.push_back(o);
      ++current_instr_ptr.addr;
    } break;
    case IRTag::LTE: {
      VMInstr o = build(where, VMTag::LTE);
      instructions.push_back(o);
      ++current_instr_ptr.addr;
    } break;
    case IRTag::GT: {
      VMInstr o = build(where, VMTag::GT);
      instructions.push_back(o);
      ++current_instr_ptr.addr;
    } break;
    case IRTag::GTE: {
      VMInstr o = build(where, VMTag::GTE);
      instructions.push_back(o);
      ++current_instr_ptr.addr;
    } break;
    case IRTag::EQ: {
      VMInstr o = build(where, VMTag::EQ);
      instructions.push_back(o);
      ++current_instr_ptr.addr;
    } break;
    case IRTag::NEQ: {
      VMInstr o = build(where, VMTag::NEQ);
      instructions.push_back(o);
      ++current_instr_ptr.addr;
    } break;
    case IRTag::RETURN: {
      VMInstr o = build(where, VMTag::RETURN);
      instructions.push_back(o);
      ++current_instr_ptr.addr;
    } break;
    case IRTag::DUP: {
      VMInstr o = build(where, VMTag::DUP);
      instructions.push_back(o);
      ++current_instr_ptr.addr;
    } break;
    case IRTag::TYPEOF: {
      VMInstr o = build(where, VMTag::TYPEOF);
      instructions.push_back(o);
      ++current_instr_ptr.addr;
    } break;
    case IRTag::POP: {
      VMInstr o = build(where, VMTag::POP);
      instructions.push_back(o);
      ++current_instr_ptr.addr;
    } break;
    case IRTag::ISTRUE: {
      VMInstr o = build(where, VMTag::ISTRUE);
      instructions.push_back(o);
      ++current_instr_ptr.addr;
    } break;
    case IRTag::CALL: {
      VMInstr o;
      o.where = instr.where;
      o.as.INSTRADDR = label_map.at(instr.as.VAR.index);
      o.tag = VMTag::CALL;
      o.extra.args = 0;
      instructions.push_back(o);
      ++current_instr_ptr.addr;
    } break;
    case IRTag::VMCALL: {
      VMInstr o = build_vmcall(instr.where, instr.extra.args);
      instructions.push_back(o);
      ++current_instr_ptr.addr;
    } break;
    case IRTag::FUNCTION_START: {
      VMInstr o = build_init_frame(instr.where, instr.extra.locals);
      instructions.push_back(o);
      label_map[instr.as.VAR.index] = current_instr_ptr;
      global_stkvars_ptr = local_stkvars_ptr;
      local_stkvars_ptr.addr = 0;
      ++current_instr_ptr.addr;
    } break;
    case IRTag::FUNCTION_END: {
      local_stkvars_ptr = global_stkvars_ptr;
    } break;
    case IRTag::ISTRUE_PEEK_JMPIF: {
      InstrAddr addr = label_map.at(instr.as.LABEL.idx);
      VMInstr o = build_jump(where, VMTag::ISTRUE_PEEK_JMPIF, addr);
      instructions.push_back(o);
      ++current_instr_ptr.addr;
    } break;
    case IRTag::ISTRUE_PEEK_JMPIFN: {
      InstrAddr addr = label_map.at(instr.as.LABEL.idx);
      VMInstr o = build_jump(where, VMTag::ISTRUE_PEEK_JMPIFN, addr);
      instructions.push_back(o);
      ++current_instr_ptr.addr;
    } break;
    case IRTag::JMP: {
      InstrAddr addr = label_map.at(instr.as.LABEL.idx);
      VMInstr o = build_jump(where, VMTag::JMP, addr);
      instructions.push_back(o);
      ++current_instr_ptr.addr;
    } break;
    case IRTag::JMPIFN: {
      InstrAddr addr = label_map.at(instr.as.LABEL.idx);
      VMInstr o = build_jump(where, VMTag::JMPIFN, addr);
      instructions.push_back(o);
      ++current_instr_ptr.addr;
    } break;
    case IRTag::JMPIF: {
      InstrAddr addr = label_map.at(instr.as.LABEL.idx);
      VMInstr o = build_jump(where, VMTag::JMPIF, addr);
      instructions.push_back(o);
      ++current_instr_ptr.addr;
    } break;
    case IRTag::LABEL:
      label_map[instr.as.VAR.index] = InstrAddr{(current_instr_ptr.addr + 1)};
      break;
    case IRTag::SCOPE_START:
    case IRTag::SCOPE_END:
      break;
    }
  }
  assert(current_instr_ptr.addr == instructions.size());
  return instructions;
}
