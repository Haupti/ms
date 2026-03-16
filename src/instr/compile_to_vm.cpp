#include "../../lib/asap/util.hpp"
#include "../compile_error.hpp"
#include "../ir/ir_instr.hpp"
#include "../vm/vm_instr.hpp"
#include <cassert>
#include <unordered_map>
#include <vector>

// this has the following assumptions for the IR:
// * all variable names are unique per frame
// * all labels are unique
namespace {
VMInstr build_init_template() {
  VMInstr o;
  o.where = LocationRef{0};
  o.as.NONE = false;
  o.tag = VMTag::PROGRAM_INIT;
  o.extra.globals = 0;
  return o;
}
VMInstr build_halt() {
  VMInstr o;
  o.where = LocationRef{0};
  o.as.NONE = false;
  o.tag = VMTag::HALT;
  o.extra.args = 0;
  return o;
}
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
VMInstr build_vmcall(LocationRef where, InternedString vmfn, uint16_t args) {
  VMInstr o;
  o.where = where;
  o.as.VMFN = vmfn;
  o.tag = VMTag::VMCALL;
  o.extra.args = args;
  return o;
}
VMInstr build_init_frame(LocationRef where, uint16_t locals, uint16_t args) {
  VMInstr o;
  o.where = where;
  o.as.INT = args;
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

enum class EntryTag : uint8_t {
  FN_BEGIN,
  FN_END,
  SCOPE_BEGIN,
  SCOPE_END,
  VAR,
};

struct VarTrackingEntry {
  uint64_t depth;
  InternedString name;
  StkAddr offset;
  EntryTag tag;
};

void set_labels(
    std::unordered_map<uint64_t, InstrAddr> *labels, std::vector<IRInstr> *ir) {
  // because there is a program init instruction prepended at the very beginning
  // thus all addresses must be shifted 1 up
  uint64_t addr = 1;
  uint64_t i = 0;
  for (i = 0; i < ir->size(); ++i) {
    switch (ir->at(i).tag) {
    case IRTag::PUSH_INT:
    case IRTag::PUSH_FLOAT:
    case IRTag::PUSH_ALLOC_STRING:
    case IRTag::PUSH_SYMBOL:
    case IRTag::PUSH_NONE:
    case IRTag::STORE:
    case IRTag::STORE_NEW:
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
    case IRTag::RETURN:
    case IRTag::DUP:
    case IRTag::TYPEOF:
    case IRTag::CALL:
    case IRTag::VMCALL:
    case IRTag::POP:
    case IRTag::PEEK_JMPIF:
    case IRTag::PEEK_JMPIFN:
    case IRTag::JMP:
    case IRTag::JMPIFN:
    case IRTag::JMPIF:
    case IRTag::HALT:
      ++addr;
      break;
    case IRTag::FUNCTION_END:
    case IRTag::REGISTER_ARG:
    case IRTag::SCOPE_START:
    case IRTag::SCOPE_END:
      // omitted in the output -> don't contribute to output address counter
      break;
    case IRTag::FUNCTION_START:
      (*labels)[ir->at(i).as.LABEL.idx] = InstrAddr{addr};
      ++addr;
      break;
    case IRTag::LABEL:
      (*labels)[ir->at(i).as.LABEL.idx] = InstrAddr{addr};
      // omitted in the output -> doesn't contribute to output address counter
      break;
    }
  }
}

VarTrackingEntry get_var(std::vector<VarTrackingEntry> *vars, uint64_t depth,
                         InternedString varname) {

  // search within current function
  for (auto i = vars->rbegin(); i != vars->rend(); ++i) {
    if (i->tag == EntryTag::FN_BEGIN) {
      break;
    }
    if (i->depth <= depth and i->name.index == varname.index) {
      return *i;
    }
  }
  // search within global scope
  for (auto i = vars->rbegin(); i != vars->rend(); ++i) {
    if (i->depth == 0 and i->name.index == varname.index) {
      return *i;
    }
  }
  panic("undefined variable referenced '" + resolve_interned_string(varname) +
        "'");
}

VarTrackingEntry set_var(std::vector<VarTrackingEntry> *vars, uint64_t depth,
                         InternedString varname) {

  uint64_t offset = 0;
  for (auto i = vars->rbegin(); i != vars->rend(); ++i) {
    if (i->tag == EntryTag::FN_BEGIN) {
      break;
    } else if (i->tag == EntryTag::VAR) {
      ++offset;
    }
  }
  vars->push_back(
      VarTrackingEntry{depth, varname, StkAddr{offset}, EntryTag::VAR});
  return vars->back();
}
void enter_fn(std::vector<VarTrackingEntry> *vars) {
  vars->push_back(VarTrackingEntry{0, {0}, {0}, EntryTag::FN_BEGIN});
}
void exit_fn(std::vector<VarTrackingEntry> *vars) {
  vars->push_back(VarTrackingEntry{0, {0}, {0}, EntryTag::FN_END});
}
void enter_scope(std::vector<VarTrackingEntry> *vars) {
  vars->push_back(VarTrackingEntry{0, {0}, {0}, EntryTag::SCOPE_BEGIN});
}
void exit_scope(std::vector<VarTrackingEntry> *vars) {
  vars->push_back(VarTrackingEntry{0, {0}, {0}, EntryTag::SCOPE_END});
}
uint64_t count_globals(std::vector<VarTrackingEntry> *vars) {
  uint64_t globals = 0;
  for (auto i = vars->begin(); i != vars->end(); ++i) {
    if (i->tag == EntryTag::FN_BEGIN) {
      break;
    } else if (i->tag == EntryTag::VAR) {
      ++globals;
    }
  }
  return globals;
}

} // namespace

std::vector<VMInstr> compile_to_vm(std::vector<IRInstr> ir) {
  std::vector<VMInstr> instructions;
  instructions.reserve(ir.size());
  instructions.push_back(build_init_template());

  uint64_t depth = 0;
  std::vector<VarTrackingEntry> vars;

  // label name index -> instr addr
  std::unordered_map<uint64_t, InstrAddr> labels;

  set_labels(&labels, &ir);

  IRInstr instr;
  for (uint64_t i = 0; i < ir.size(); ++i) {

    instr = ir.at(i);
    LocationRef where = instr.where;

    switch (instr.tag) {
    case IRTag::PUSH_INT: {
      auto o = build_int(where, VMTag::PUSH_INT, instr.as.INT);
      instructions.push_back(o);
    } break;
    case IRTag::PUSH_FLOAT: {
      auto o = build_float(where, VMTag::PUSH_FLOAT, instr.as.FLOAT);
      instructions.push_back(o);
    } break;
    case IRTag::PUSH_ALLOC_STRING: {
      auto o = build_string(where, VMTag::PUSH_ALLOC_STRING, instr.as.STRING);
      instructions.push_back(o);
    } break;
    case IRTag::PUSH_SYMBOL: {
      VMInstr o = build_symbol(where, VMTag::PUSH_SYMBOL, instr.as.SYMBOL);
      instructions.push_back(o);
    } break;
    case IRTag::PUSH_NONE: {
      instructions.push_back(build_none(where));
    } break;
    case IRTag::STORE: {
      VarTrackingEntry var = get_var(&vars, depth, instr.as.VAR);
      if (var.depth == 0) {
        VMInstr o = build_addr_acc(where, VMTag::STORE_GLOBAL, var.offset);
        instructions.push_back(o);
      } else {
        VMInstr o = build_addr_acc(where, VMTag::STORE, var.offset);
        instructions.push_back(o);
      }
    } break;
    case IRTag::STORE_NEW: {
      VarTrackingEntry var = set_var(&vars, depth, instr.as.VAR);
      if (depth == 0) {
        VMInstr o = build_addr_acc(where, VMTag::STORE_GLOBAL, var.offset);
        instructions.push_back(o);
      } else {
        VMInstr o = build_addr_acc(where, VMTag::STORE, var.offset);
        instructions.push_back(o);
      }
    } break;
    case IRTag::LOAD: {
      VarTrackingEntry var = get_var(&vars, depth, instr.as.VAR);
      if (var.depth == 0) {
        VMInstr o = build_addr_acc(where, VMTag::LOAD_GLOBAL, var.offset);
        instructions.push_back(o);
      } else {
        VMInstr o = build_addr_acc(where, VMTag::LOAD, var.offset);
        instructions.push_back(o);
      }
    } break;
    case IRTag::NOT: {
      instructions.push_back(build(where, VMTag::NOT));
    } break;
    case IRTag::INVERT: {
      instructions.push_back(build(where, VMTag::INVERT));
    } break;
    case IRTag::ADD: {
      instructions.push_back(build(where, VMTag::ADD));
    } break;
    case IRTag::SUB: {
      instructions.push_back(build(where, VMTag::SUB));
    } break;
    case IRTag::MUL: {
      instructions.push_back(build(where, VMTag::MUL));
    } break;
    case IRTag::DIV: {
      instructions.push_back(build(where, VMTag::DIV));
    } break;
    case IRTag::MOD: {
      instructions.push_back(build(where, VMTag::MOD));
    } break;
    case IRTag::LT: {
      instructions.push_back(build(where, VMTag::LT));
    } break;
    case IRTag::LTE: {
      instructions.push_back(build(where, VMTag::LTE));
    } break;
    case IRTag::GT: {
      instructions.push_back(build(where, VMTag::GT));
    } break;
    case IRTag::GTE: {
      instructions.push_back(build(where, VMTag::GTE));
    } break;
    case IRTag::EQ: {
      instructions.push_back(build(where, VMTag::EQ));
    } break;
    case IRTag::NEQ: {
      instructions.push_back(build(where, VMTag::NEQ));
    } break;
    case IRTag::RETURN: {
      instructions.push_back(build(where, VMTag::RETURN));
    } break;
    case IRTag::DUP: {
      instructions.push_back(build(where, VMTag::DUP));
    } break;
    case IRTag::TYPEOF: {
      instructions.push_back(build(where, VMTag::TYPEOF));
    } break;
    case IRTag::POP: {
      instructions.push_back(build(where, VMTag::POP));
    } break;
    case IRTag::CALL: {
      if (labels.count(instr.as.VAR.index) == 0) {
        throw compile_error(instr.where,
                            "undefined function reference '" +
                                resolve_interned_string(instr.as.VAR) + "'");
      }
      VMInstr o;
      o.where = instr.where;
      o.as.INSTRADDR = labels.at(instr.as.VAR.index);
      o.tag = VMTag::CALL;
      o.extra.args = 0;
      instructions.push_back(o);
    } break;
    case IRTag::VMCALL: {
      instructions.push_back(
          build_vmcall(instr.where, instr.as.VAR, instr.extra.args));
    } break;
    case IRTag::FUNCTION_START: {
      enter_fn(&vars);
      ++depth;
      VMInstr o =
          build_init_frame(instr.where, instr.extra.locals, instr.extra.args);
      instructions.push_back(o);
    } break;
    case IRTag::REGISTER_ARG: {
      set_var(&vars, depth, instr.as.VAR);
      // no output instruction, just register for offsets
    } break;
    case IRTag::FUNCTION_END:
      exit_fn(&vars);
      --depth;
      break;
    case IRTag::PEEK_JMPIF: {
      InstrAddr addr = labels.at(instr.as.LABEL.idx);
      VMInstr o = build_jump(where, VMTag::PEEK_JMPIF, addr);
      instructions.push_back(o);
    } break;
    case IRTag::PEEK_JMPIFN: {
      InstrAddr addr = labels.at(instr.as.LABEL.idx);
      VMInstr o = build_jump(where, VMTag::PEEK_JMPIFN, addr);
      instructions.push_back(o);
    } break;
    case IRTag::JMP: {
      InstrAddr addr = labels.at(instr.as.LABEL.idx);
      VMInstr o = build_jump(where, VMTag::JMP, addr);
      instructions.push_back(o);
    } break;
    case IRTag::JMPIFN: {
      InstrAddr addr = labels.at(instr.as.LABEL.idx);
      VMInstr o = build_jump(where, VMTag::JMPIFN, addr);
      instructions.push_back(o);
    } break;
    case IRTag::JMPIF: {
      InstrAddr addr = labels.at(instr.as.LABEL.idx);
      VMInstr o = build_jump(where, VMTag::JMPIF, addr);
      instructions.push_back(o);
    } break;
    case IRTag::LABEL:
      // handled in pre-flight
      break;
    case IRTag::SCOPE_START:
      enter_scope(&vars);
      ++depth;
      break;
    case IRTag::SCOPE_END:
      exit_scope(&vars);
      --depth;
      break;
    case IRTag::HALT:
      instructions.push_back(build_halt());
      break;
    }
  }

  instructions.at(0).extra.globals = count_globals(&vars);
  instructions.push_back(build_halt());
  return instructions;
}
