#pragma once

#include "../preprocessor/token.hpp"
#include "ast_node.hpp"
#include <vector>
std::vector<ASTNode> parse(const std::vector<Token> program);
