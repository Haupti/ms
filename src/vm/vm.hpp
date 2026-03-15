#pragma once

#include "vm_instr.hpp"
#include <vector>
int run(std::vector<VMInstr> instrs, const std::vector<std::string> &msl_args);
