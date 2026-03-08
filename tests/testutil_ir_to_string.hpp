#pragma once
#include "../src/ir/ir_instr.hpp"
#include <string>

inline std::string ir_to_string(const IRInstr &instr) {
  switch (instr.tag) {
  case IRTag::PUSH_INT:
    return "PUSH_INT " + std::to_string(instr.as.INT);
  case IRTag::PUSH_FLOAT:
    return "PUSH_FLOAT " + std::to_string(instr.as.FLOAT);
  case IRTag::PUSH_ALLOC_STRING:
    return "PUSH_ALLOC_STRING " + resolve_interned_string(instr.as.STRING);
  case IRTag::PUSH_SYMBOL:
    return "PUSH_SYMBOL " + resolve_symbol(instr.as.SYMBOL);
  case IRTag::STORE:
    return "STORE " + resolve_interned_string(instr.as.VAR);
  case IRTag::LOAD:
    return "LOAD " + resolve_interned_string(instr.as.VAR);
  case IRTag::NOT:
    return "NOT";
  case IRTag::INVERT:
    return "INVERT";
  case IRTag::ADD:
    return "ADD";
  case IRTag::SUB:
    return "SUB";
  case IRTag::MUL:
    return "MUL";
  case IRTag::DIV:
    return "DIV";
  case IRTag::MOD:
    return "MOD";
  case IRTag::LT:
    return "LT";
  case IRTag::LTE:
    return "LTE";
  case IRTag::GT:
    return "GT";
  case IRTag::GTE:
    return "GTE";
  case IRTag::EQ:
    return "EQ";
  case IRTag::NEQ:
    return "NEQ";
  case IRTag::ISTRUE:
    return "ISTRUE";
  case IRTag::POP:
    return "POP";
  case IRTag::JMPIFN:
    return "JMPIFN " + resolve_label(instr.as.LABEL);
  case IRTag::LABEL:
    return "LABEL " + resolve_label(instr.as.LABEL);
  case IRTag::JMP:
    return "JMP " + resolve_label(instr.as.LABEL);
  case IRTag::PUSH_NONE:
    return "PUSH_NONE";
  case IRTag::RETURN:
    return "RETURN";
  case IRTag::CALL:
    return "CALL " + resolve_interned_string(instr.as.VAR);
  case IRTag::VMCALL:
    return "VMCALL " + resolve_interned_string(instr.as.VAR);
  case IRTag::FUNCTION_START:
    return "FUNCTION_START " + resolve_interned_string(instr.as.VAR);
  case IRTag::ISTRUE_PEEK_JMPIF:
    return "ISTRUE_PEEK_JMPIF " + resolve_label(instr.as.LABEL);
  case IRTag::ISTRUE_PEEK_JMPIFN:
    return "ISTRUE_PEEK_JMPIFN " + resolve_label(instr.as.LABEL);
  case IRTag::JMPIF:
    return "JMPIF " + resolve_label(instr.as.LABEL);
  case IRTag::DUP:
    return "DUP";
  case IRTag::TYPEOF:
    return "TYPEOF";
  case IRTag::STORE_NEW:
    return "STORE_NEW " + resolve_interned_string(instr.as.VAR);
  case IRTag::FUNCTION_END:
    return "FUNCTION_END";
  case IRTag::SCOPE_START:
    return "SCOPE_START";
  case IRTag::SCOPE_END:
    return "SCOPE_END";
  case IRTag::HALT:
    return "HALT";
  }
}
