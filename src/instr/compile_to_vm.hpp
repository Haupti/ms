#pragma once

#include <vector>
#include "../vm/vm_instr.hpp"
#include "../ir/ir_instr.hpp"
std::vector<VMInstr> compile_to_vm(std::vector<IRInstr> ir);
