#include "../ir/ir_instr.hpp"
#include "../vm/vm_instr.hpp"
#include <unordered_map>
#include <vector>

std::vector<VMInstr> compile_to_vm(std::vector<IRInstr> ir) {
  std::vector<VMInstr> out;
  out.reserve(ir.size());

  // uint64_t of interned string
  std::unordered_map<uint64_t, StkAddr> var_map;
  StkAddr current_stack_ptr;

  for (auto instr : ir) {
    switch (instr.tag) {
    case IRTag::PUSH_INT: {
      VMInstr o;
      o.where = instr.where;
      o.as.INT = instr.as.INT;
      o.tag = VMTag::PUSH_INT;
      out.push_back(o);
    } break;
    case IRTag::PUSH_FLOAT: {
      VMInstr o;
      o.where = instr.where;
      o.as.FLOAT = instr.as.FLOAT;
      o.tag = VMTag::PUSH_FLOAT;
      out.push_back(o);
    } break;
    case IRTag::PUSH_ALLOC_STRING: {
      VMInstr o;
      o.where = instr.where;
      o.as.STRING = instr.as.STRING;
      o.tag = VMTag::PUSH_ALLOC_STRING;
      out.push_back(o);
    } break;
    case IRTag::PUSH_SYMBOL: {
      VMInstr o;
      o.where = instr.where;
      o.as.SYMBOL = instr.as.SYMBOL;
      o.tag = VMTag::PUSH_SYMBOL;
      out.push_back(o);
    } break;
    case IRTag::PUSH_NONE: {
      VMInstr o;
      o.where = instr.where;
      o.as.NONE = instr.as.NONE;
      o.tag = VMTag::PUSH_NONE;
      out.push_back(o);
    } break;
    case IRTag::STORE: {
      VMInstr o;
      o.where = instr.where;
      o.as.VAR = var_map.at(instr.as.VAR.index);
      o.tag = VMTag::STORE;
      out.push_back(o);
    } break;
    case IRTag::ALLOC_STORE: {
      var_map.at(instr.as.VAR.index) = current_stack_ptr;
      current_stack_ptr.addr += 1;

      VMInstr o;
      o.where = instr.where;
      o.as.VAR = var_map.at(instr.as.VAR.index);
      o.tag = VMTag::STORE;
      out.push_back(o);
    } break;
    case IRTag::LOAD: {
      VMInstr o;
      o.where = instr.where;
      o.as.VAR = var_map.at(instr.as.VAR.index);
      o.tag = VMTag::LOAD;
      out.push_back(o);
    } break;
    case IRTag::NOT: {
      VMInstr o;
      o.where = instr.where;
      o.as.NONE = false;
      o.tag = VMTag::NOT;
      out.push_back(o);
    } break;
    case IRTag::INVERT: {
      VMInstr o;
      o.where = instr.where;
      o.as.NONE = false;
      o.tag = VMTag::INVERT;
      out.push_back(o);
    } break;
    case IRTag::ADD: {
      VMInstr o;
      o.where = instr.where;
      o.as.NONE = false;
      o.tag = VMTag::ADD;
      out.push_back(o);
    } break;
    case IRTag::SUB: {
      VMInstr o;
      o.where = instr.where;
      o.as.NONE = false;
      o.tag = VMTag::SUB;
      out.push_back(o);
    } break;
    case IRTag::MUL: {
      VMInstr o;
      o.where = instr.where;
      o.as.NONE = false;
      o.tag = VMTag::MUL;
      out.push_back(o);
    } break;
    case IRTag::DIV: {
      VMInstr o;
      o.where = instr.where;
      o.as.NONE = false;
      o.tag = VMTag::DIV;
      out.push_back(o);
    } break;
    case IRTag::MOD: {
      VMInstr o;
      o.where = instr.where;
      o.as.NONE = false;
      o.tag = VMTag::MOD;
      out.push_back(o);
    } break;
    case IRTag::LT: {
      VMInstr o;
      o.where = instr.where;
      o.as.NONE = false;
      o.tag = VMTag::LT;
      out.push_back(o);
    } break;
    case IRTag::LTE: {
      VMInstr o;
      o.where = instr.where;
      o.as.NONE = false;
      o.tag = VMTag::LTE;
      out.push_back(o);
    } break;
    case IRTag::GT: {
      VMInstr o;
      o.where = instr.where;
      o.as.NONE = false;
      o.tag = VMTag::GT;
      out.push_back(o);
    } break;
    case IRTag::GTE: {
      VMInstr o;
      o.where = instr.where;
      o.as.NONE = false;
      o.tag = VMTag::GTE;
      out.push_back(o);
    } break;
    case IRTag::EQ: {
      VMInstr o;
      o.where = instr.where;
      o.as.NONE = false;
      o.tag = VMTag::EQ;
      out.push_back(o);
    } break;
    case IRTag::NEQ: {
      VMInstr o;
      o.where = instr.where;
      o.as.NONE = false;
      o.tag = VMTag::NEQ;
      out.push_back(o);
    } break;
    case IRTag::STR_CONCAT: {
      // TODO vm function call
    } break;
    case IRTag::RETURN: {
      VMInstr o;
      o.where = instr.where;
      o.as.NONE = false;
      o.tag = VMTag::RETURN;
      out.push_back(o);
    } break;
    case IRTag::DUP: {
      VMInstr o;
      o.where = instr.where;
      o.as.NONE = false;
      o.tag = VMTag::DUP;
      out.push_back(o);
    } break;
    case IRTag::TYPEOF: {
      VMInstr o;
      o.where = instr.where;
      o.as.NONE = false;
      o.tag = VMTag::TYPEOF;
      out.push_back(o);
    } break;
    case IRTag::CALL:
      // TODO
    case IRTag::VMCALL:
      // TODO
    case IRTag::INIT_FRAME:
      // TODO
    case IRTag::INIT_ANON_FRAME:
      // TODO
    case IRTag::DESTROY_FRAME:
      // TODO
    case IRTag::POP:
      // TODO
    case IRTag::ISTRUE:
      // TODO
    case IRTag::ISTRUE_PEEK_JMPIF:
      // TODO
    case IRTag::ISTRUE_PEEK_JMPIFN:
      // TODO
    case IRTag::JMP:
      // TODO
    case IRTag::JMPIFN:
      // TODO
    case IRTag::JMPIF:
      // TODO
    case IRTag::LABEL:
      // TODO
      break;
    }
  }
}
