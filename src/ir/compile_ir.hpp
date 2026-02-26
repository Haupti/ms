#pragma once

#include "../parser/node.hpp"
#include "ir_instr.hpp"
#include <vector>

std::vector<IRInstr> compile_ir(nodes ns);
