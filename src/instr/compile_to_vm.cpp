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

std::vector<VMInstr> compile_to_vm(std::vector<IRInstr> ir) {
  std::vector<VMInstr> instructions;
  instructions.reserve(ir.size());

  // uint64_t of interned string
  std::unordered_map<uint64_t, StkAddr> global_var_map;
  // uint64_t of interned string
  std::unordered_map<uint64_t, StkAddr> local_var_map;
  // uint64_t of interned string
  std::unordered_map<uint64_t, InstrAddr> label_map;
  StkAddr global_stkvars_ptr;
  StkAddr local_stkvars_ptr;
  InstrAddr current_instr_ptr;

  for (auto instr : ir) {
    switch (instr.tag) {
    case IRTag::PUSH_INT: {
      VMInstr o;
      o.where = instr.where;
      o.as.INT = instr.as.INT;
      o.tag = VMTag::PUSH_INT;
      o.extra.args = 0;
      instructions.push_back(o);
      ++current_instr_ptr.addr;
    } break;
    case IRTag::PUSH_FLOAT: {
      VMInstr o;
      o.where = instr.where;
      o.as.FLOAT = instr.as.FLOAT;
      o.tag = VMTag::PUSH_FLOAT;
      o.extra.args = 0;
      instructions.push_back(o);
      ++current_instr_ptr.addr;
    } break;
    case IRTag::PUSH_ALLOC_STRING: {
      VMInstr o;
      o.where = instr.where;
      o.as.STRING = instr.as.STRING;
      o.tag = VMTag::PUSH_ALLOC_STRING;
      o.extra.args = 0;
      instructions.push_back(o);
      ++current_instr_ptr.addr;
    } break;
    case IRTag::PUSH_SYMBOL: {
      VMInstr o;
      o.where = instr.where;
      o.as.SYMBOL = instr.as.SYMBOL;
      o.tag = VMTag::PUSH_SYMBOL;
      o.extra.args = 0;
      instructions.push_back(o);
      ++current_instr_ptr.addr;
    } break;
    case IRTag::PUSH_NONE: {
      VMInstr o;
      o.where = instr.where;
      o.as.NONE = instr.as.NONE;
      o.tag = VMTag::PUSH_NONE;
      o.extra.args = 0;
      instructions.push_back(o);
      ++current_instr_ptr.addr;
    } break;
    case IRTag::STORE: {
      VMInstr o;
      o.where = instr.where;
      o.as.STKADR = local_var_map.at(instr.as.VAR.index);
      o.tag = VMTag::STORE;
      o.extra.args = 0;
      instructions.push_back(o);
      ++current_instr_ptr.addr;
    } break;
    case IRTag::STORE_GLOBAL: {
      VMInstr o;
      o.where = instr.where;
      o.as.STKADR = global_var_map.at(instr.as.VAR.index);
      o.tag = VMTag::STORE;
      o.extra.args = 0;
      instructions.push_back(o);
      ++current_instr_ptr.addr;
    } break;
    case IRTag::STORE_NEW: {
      local_var_map.at(instr.as.VAR.index) = local_stkvars_ptr;
      local_stkvars_ptr.addr += 1;

      VMInstr o;
      o.where = instr.where;
      o.as.STKADR = local_var_map.at(instr.as.VAR.index);
      o.tag = VMTag::STORE;
      o.extra.args = 0;
      instructions.push_back(o);
      ++current_instr_ptr.addr;
    } break;
    case IRTag::STORE_NEW_GLOBAL: {
      global_var_map.at(instr.as.VAR.index) = global_stkvars_ptr;
      local_stkvars_ptr.addr += 1;

      VMInstr o;
      o.where = instr.where;
      o.as.STKADR = global_var_map.at(instr.as.VAR.index);
      o.tag = VMTag::STORE;
      o.extra.args = 0;
      instructions.push_back(o);
      ++current_instr_ptr.addr;
    } break;
    case IRTag::LOAD: {
      VMInstr o;
      o.where = instr.where;
      o.as.STKADR = local_var_map.at(instr.as.VAR.index);
      o.tag = VMTag::LOAD;
      o.extra.args = 0;
      instructions.push_back(o);
      ++current_instr_ptr.addr;
    } break;
    case IRTag::LOAD_GLOBAL: {
      VMInstr o;
      o.where = instr.where;
      o.as.STKADR = global_var_map.at(instr.as.VAR.index);
      o.tag = VMTag::LOAD;
      o.extra.args = 0;
      instructions.push_back(o);
      ++current_instr_ptr.addr;
    } break;
    case IRTag::NOT: {
      VMInstr o;
      o.where = instr.where;
      o.as.NONE = false;
      o.tag = VMTag::NOT;
      o.extra.args = 0;
      instructions.push_back(o);
      ++current_instr_ptr.addr;
    } break;
    case IRTag::INVERT: {
      VMInstr o;
      o.where = instr.where;
      o.as.NONE = false;
      o.tag = VMTag::INVERT;
      o.extra.args = 0;
      instructions.push_back(o);
      ++current_instr_ptr.addr;
    } break;
    case IRTag::ADD: {
      VMInstr o;
      o.where = instr.where;
      o.as.NONE = false;
      o.tag = VMTag::ADD;
      o.extra.args = 0;
      instructions.push_back(o);
      ++current_instr_ptr.addr;
    } break;
    case IRTag::SUB: {
      VMInstr o;
      o.where = instr.where;
      o.as.NONE = false;
      o.tag = VMTag::SUB;
      o.extra.args = 0;
      instructions.push_back(o);
      ++current_instr_ptr.addr;
    } break;
    case IRTag::MUL: {
      VMInstr o;
      o.where = instr.where;
      o.as.NONE = false;
      o.tag = VMTag::MUL;
      o.extra.args = 0;
      instructions.push_back(o);
      ++current_instr_ptr.addr;
    } break;
    case IRTag::DIV: {
      VMInstr o;
      o.where = instr.where;
      o.as.NONE = false;
      o.tag = VMTag::DIV;
      o.extra.args = 0;
      instructions.push_back(o);
      ++current_instr_ptr.addr;
    } break;
    case IRTag::MOD: {
      VMInstr o;
      o.where = instr.where;
      o.as.NONE = false;
      o.tag = VMTag::MOD;
      o.extra.args = 0;
      instructions.push_back(o);
      ++current_instr_ptr.addr;
    } break;
    case IRTag::LT: {
      VMInstr o;
      o.where = instr.where;
      o.as.NONE = false;
      o.tag = VMTag::LT;
      o.extra.args = 0;
      instructions.push_back(o);
      ++current_instr_ptr.addr;
    } break;
    case IRTag::LTE: {
      VMInstr o;
      o.where = instr.where;
      o.as.NONE = false;
      o.tag = VMTag::LTE;
      o.extra.args = 0;
      instructions.push_back(o);
      ++current_instr_ptr.addr;
    } break;
    case IRTag::GT: {
      VMInstr o;
      o.where = instr.where;
      o.as.NONE = false;
      o.tag = VMTag::GT;
      o.extra.args = 0;
      instructions.push_back(o);
      ++current_instr_ptr.addr;
    } break;
    case IRTag::GTE: {
      VMInstr o;
      o.where = instr.where;
      o.as.NONE = false;
      o.tag = VMTag::GTE;
      o.extra.args = 0;
      instructions.push_back(o);
      ++current_instr_ptr.addr;
    } break;
    case IRTag::EQ: {
      VMInstr o;
      o.where = instr.where;
      o.as.NONE = false;
      o.tag = VMTag::EQ;
      o.extra.args = 0;
      instructions.push_back(o);
      ++current_instr_ptr.addr;
    } break;
    case IRTag::NEQ: {
      VMInstr o;
      o.where = instr.where;
      o.as.NONE = false;
      o.tag = VMTag::NEQ;
      o.extra.args = 0;
      instructions.push_back(o);
      ++current_instr_ptr.addr;
    } break;
    case IRTag::RETURN: {
      VMInstr o;
      o.where = instr.where;
      o.as.NONE = false;
      o.tag = VMTag::RETURN;
      o.extra.args = 0;
      instructions.push_back(o);
      ++current_instr_ptr.addr;
    } break;
    case IRTag::DUP: {
      VMInstr o;
      o.where = instr.where;
      o.as.NONE = false;
      o.tag = VMTag::DUP;
      o.extra.args = 0;
      instructions.push_back(o);
      ++current_instr_ptr.addr;
    } break;
    case IRTag::TYPEOF: {
      VMInstr o;
      o.where = instr.where;
      o.as.NONE = false;
      o.tag = VMTag::TYPEOF;
      o.extra.args = 0;
      instructions.push_back(o);
      ++current_instr_ptr.addr;
    } break;
    case IRTag::POP: {
      VMInstr o;
      o.where = instr.where;
      o.as.NONE = false;
      o.tag = VMTag::POP;
      o.extra.args = 0;
      instructions.push_back(o);
      ++current_instr_ptr.addr;
    } break;
    case IRTag::ISTRUE: {
      VMInstr o;
      o.where = instr.where;
      o.as.NONE = false;
      o.tag = VMTag::ISTRUE;
      o.extra.args = 0;
      instructions.push_back(o);
      ++current_instr_ptr.addr;
    } break;
    case IRTag::CALL: {
      VMInstr o;
      o.where = instr.where;
      o.as.NONE = false;
      o.tag = VMTag::CALL;
      o.extra.args = 0;
      instructions.push_back(o);
      ++current_instr_ptr.addr;
    } break;
    case IRTag::VMCALL: {
      VMInstr o;
      o.where = instr.where;
      o.as.NONE = false;
      o.tag = VMTag::VMCALL;
      o.extra.args = instr.extra.args;
      instructions.push_back(o);
      ++current_instr_ptr.addr;
    } break;
    case IRTag::INIT_FRAME: {
      VMInstr o;
      o.where = instr.where;
      o.as.NONE = false;
      o.tag = VMTag::INIT_FRAME;
      o.extra.locals = instr.extra.locals;
      instructions.push_back(o);
      global_stkvars_ptr = local_stkvars_ptr;
      local_stkvars_ptr.addr = 0;
      ++current_instr_ptr.addr;
    } break;
    case IRTag::FRAME_END: {
      local_stkvars_ptr = global_stkvars_ptr;
    } break;
    case IRTag::ISTRUE_PEEK_JMPIF: {
      VMInstr o;
      o.where = instr.where;
      o.as.INSTRADDR = label_map.at(instr.as.LABEL.idx);
      o.tag = VMTag::ISTRUE_PEEK_JMPIF;
      o.extra.args = 0;
      instructions.push_back(o);
      ++current_instr_ptr.addr;
    } break;
    case IRTag::ISTRUE_PEEK_JMPIFN: {
      VMInstr o;
      o.where = instr.where;
      o.as.INSTRADDR = label_map.at(instr.as.LABEL.idx);
      o.tag = VMTag::ISTRUE_PEEK_JMPIFN;
      o.extra.args = 0;
      instructions.push_back(o);
      ++current_instr_ptr.addr;
    } break;
    case IRTag::JMP: {
      VMInstr o;
      o.where = instr.where;
      o.as.INSTRADDR = label_map.at(instr.as.LABEL.idx);
      o.tag = VMTag::JMP;
      o.extra.args = 0;
      instructions.push_back(o);
      ++current_instr_ptr.addr;
    } break;
    case IRTag::JMPIFN: {
      VMInstr o;
      o.where = instr.where;
      o.as.INSTRADDR = label_map.at(instr.as.LABEL.idx);
      o.tag = VMTag::JMPIFN;
      o.extra.args = 0;
      instructions.push_back(o);
      ++current_instr_ptr.addr;
    } break;
    case IRTag::JMPIF: {
      VMInstr o;
      o.where = instr.where;
      o.as.INSTRADDR = label_map.at(instr.as.LABEL.idx);
      o.tag = VMTag::JMPIF;
      o.extra.args = 0;
      instructions.push_back(o);
      ++current_instr_ptr.addr;
    } break;
    case IRTag::LABEL:
      label_map[instr.as.VAR.index] = InstrAddr{(current_instr_ptr.addr + 1)};
      break;
    }
  }
  assert(current_instr_ptr.addr == instructions.size());
  return instructions;
}
