#pragma once

#include "../preprocessor/token.hpp"
#include "node.hpp"
#include <vector>

nodes parse(const std::vector<Token> &program);
