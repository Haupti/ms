#include "../ir/ir_instr.hpp"
#include "../vm/vm_instr.hpp"
#include <vector>

std::vector<VMInstr> compile_to_vm(std::vector<IRInstr> ir) {
  std::vector<VMInstr> out;
  out.reserve(ir.size());

  for (auto instr : ir) {
    switch (instr.tag) {
    case IRTag::PUSH_INT: {
      VMInstr o;
      o.where = instr.where;
      o.as.INT = instr.as.INT;
    } break;
    case IRTag::PUSH_FLOAT:
    case IRTag::PUSH_ALLOC_STRING:
    case IRTag::PUSH_SYMBOL:
    case IRTag::PUSH_NONE:
    case IRTag::STORE:
    case IRTag::ALLOC_STORE:
    case IRTag::LOAD:
    case IRTag::NOT:
    case IRTag::INVERT:
    case IRTag::ADD:
    case IRTag::SUB:
    case IRTag::MUL:
    case IRTag::DIV:
    case IRTag::MOD:
    case IRTag::LT:
    case IRTag::LTE:
    case IRTag::GT:
    case IRTag::GTE:
    case IRTag::EQ:
    case IRTag::NEQ:
    case IRTag::STR_CONCAT:
    case IRTag::RETURN:
    case IRTag::DUP:
    case IRTag::TYPEOF:
    case IRTag::CALL:
    case IRTag::VMCALL:
    case IRTag::INIT_FRAME:
    case IRTag::INIT_ANON_FRAME:
    case IRTag::DESTROY_FRAME:
    case IRTag::POP:
    case IRTag::ISTRUE:
    case IRTag::ISTRUE_PEEK_JMPIF:
    case IRTag::ISTRUE_PEEK_JMPIFN:
    case IRTag::JMP:
    case IRTag::JMPIFN:
    case IRTag::JMPIF:
    case IRTag::LABEL:
      break;
    }
  }
}
