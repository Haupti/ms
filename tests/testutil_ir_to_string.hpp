#pragma once
#include "../src/ir/ir_instr.hpp"
#include <string>

inline std::string ir_to_string(const IRInstr &instr) {
  switch (instr.tag) {
  case IRTag::PUSH_INT:
    return "[" + location_to_string(instr.where) + "] PUSH_INT " +
           std::to_string(instr.as.INT);
  case IRTag::PUSH_FLOAT:
    return "[" + location_to_string(instr.where) + "] PUSH_FLOAT " +
           std::to_string(instr.as.FLOAT);
  case IRTag::PUSH_ALLOC_STRING:
    return "[" + location_to_string(instr.where) + "] PUSH_ALLOC_STRING " +
           resolve_interned_string(instr.as.STRING);
  case IRTag::PUSH_SYMBOL:
    return "[" + location_to_string(instr.where) + "] PUSH_SYMBOL " +
           resolve_symbol(instr.as.SYMBOL);
  case IRTag::STORE:
    return "[" + location_to_string(instr.where) + "] STORE " +
           resolve_interned_string(instr.as.VAR);
  case IRTag::LOAD:
    return "[" + location_to_string(instr.where) + "] LOAD " +
           resolve_interned_string(instr.as.VAR);
  case IRTag::NOT:
    return "[" + location_to_string(instr.where) + "] NOT";
  case IRTag::INVERT:
    return "[" + location_to_string(instr.where) + "] INVERT";
  case IRTag::ADD:
    return "[" + location_to_string(instr.where) + "] ADD";
  case IRTag::SUB:
    return "[" + location_to_string(instr.where) + "] SUB";
  case IRTag::MUL:
    return "[" + location_to_string(instr.where) + "] MUL";
  case IRTag::DIV:
    return "[" + location_to_string(instr.where) + "] DIV";
  case IRTag::MOD:
    return "[" + location_to_string(instr.where) + "] MOD";
  case IRTag::LT:
    return "[" + location_to_string(instr.where) + "] LT";
  case IRTag::LTE:
    return "[" + location_to_string(instr.where) + "] LTE";
  case IRTag::GT:
    return "[" + location_to_string(instr.where) + "] GT";
  case IRTag::GTE:
    return "[" + location_to_string(instr.where) + "] GTE";
  case IRTag::EQ:
    return "[" + location_to_string(instr.where) + "] EQ";
  case IRTag::NEQ:
    return "[" + location_to_string(instr.where) + "] NEQ";
  case IRTag::STR_CONCAT:
    return "[" + location_to_string(instr.where) + "] STR_CONCAT";
  case IRTag::AND:
    return "[" + location_to_string(instr.where) + "] AND";
  case IRTag::OR:
    return "[" + location_to_string(instr.where) + "] OR";
  case IRTag::JMPIFN:
    return "[" + location_to_string(instr.where) + "] JMPIFN" +
           resolve_label(instr.as.LABEL);
  case IRTag::LABEL:
    return "[" + location_to_string(instr.where) + "] LABEL" +
           resolve_label(instr.as.LABEL);
    break;
  }
}
