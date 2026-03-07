#pragma once
#include "../src/vm/vm_instr.hpp"
#include <string>

inline std::string vminstr_to_string(uint64_t pos, const VMInstr &instr) {
  std::string str = std::to_string(pos) + " ";
  switch (instr.tag) {
  case VMTag::PUSH_INT:
    str += "PUSH_INT " + std::to_string(instr.as.INT);
    break;
  case VMTag::PUSH_FLOAT:
    str += "PUSH_FLOAT " + std::to_string(instr.as.FLOAT);
    break;
  case VMTag::PUSH_ALLOC_STRING:
    str +=
        "PUSH_ALLOC_STRING '" + resolve_interned_string(instr.as.STRING) + "'";
    break;
  case VMTag::PUSH_SYMBOL:
    str += "PUSH_SYMBOL " + resolve_symbol(instr.as.SYMBOL);
    break;
  case VMTag::PUSH_NONE:
    str += "PUSH_NONE ";
    break;
  case VMTag::STORE:
    str += "STORE mem(" + std::to_string(instr.as.STKADDR.addr) + ")";
    break;
  case VMTag::STORE_GLOBAL:
    str += "STORE_GLOBAL mem(" + std::to_string(instr.as.STKADDR.addr) + ")";
    break;
  case VMTag::LOAD:
    str += "LOAD mem(" + std::to_string(instr.as.STKADDR.addr) + ")";
    break;
  case VMTag::LOAD_GLOBAL:
    str += "LOAD_GLOBAL mem(" + std::to_string(instr.as.STKADDR.addr) + ")";
    break;
  case VMTag::NOT:
    str += "NOT";
    break;
  case VMTag::INVERT:
    str += "INVERT";
    break;
  case VMTag::ADD:
    str += "ADD";
    break;
  case VMTag::SUB:
    str += "SUB";
    break;
  case VMTag::MUL:
    str += "MUL";
    break;
  case VMTag::DIV:
    str += "DIV";
    break;
  case VMTag::MOD:
    str += "MOD";
    break;
  case VMTag::LT:
    str += "LT";
    break;
  case VMTag::LTE:
    str += "LTE";
    break;
  case VMTag::GT:
    str += "GT";
    break;
  case VMTag::GTE:
    str += "GTE";
    break;
  case VMTag::EQ:
    str += "EQ";
    break;
  case VMTag::NEQ:
    str += "NEQ";
    break;
  case VMTag::DUP:
    str += "DUP";
    break;
  case VMTag::TYPEOF:
    str += "TYPEOF";
    break;
  case VMTag::CALL:
    str += "CALL addr(" + std::to_string(instr.as.INSTRADDR.addr) + ")";
    break;
  case VMTag::VMCALL:
    str += "VMCALL " + resolve_interned_string(instr.as.VMFN) + " " +
           std::to_string(instr.extra.args);
    break;
  case VMTag::INIT_FRAME:
    str += "INIT_FRAME " + std::to_string(instr.extra.locals);
    break;
  case VMTag::RETURN:
    str += "RETURN";
    break;
  case VMTag::POP:
    str += "POP";
    break;
  case VMTag::ISTRUE:
    str += "ISTRUE";
    break;
  case VMTag::ISTRUE_PEEK_JMPIF:
    str += "ISTRUE_PEEK_JMPIF addr(" + std::to_string(instr.as.INSTRADDR.addr) +
           ")";
    break;
  case VMTag::ISTRUE_PEEK_JMPIFN:
    str += "ISTRUE_PEEK_JMPIFN addr(" +
           std::to_string(instr.as.INSTRADDR.addr) + ")";
    break;
  case VMTag::JMP:
    str += "JMP addr(" + std::to_string(instr.as.INSTRADDR.addr) + ")";
    break;
  case VMTag::JMPIFN:
    str += "JMPIFN addr(" + std::to_string(instr.as.INSTRADDR.addr) + ")";
    break;
  case VMTag::JMPIF:
    str += "JMPIF addr(" + std::to_string(instr.as.INSTRADDR.addr) + ")";
    break;
  case VMTag::PROGRAM_INIT:
    str += "PROGRAM_INIT " + std::to_string(instr.extra.globals);
    break;
  case VMTag::HALT:
    str += "HALT";
    break;
  }
  return str;
}
