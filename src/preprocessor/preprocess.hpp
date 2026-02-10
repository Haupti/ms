#pragma once

#include "preprocessor_token.hpp"
#include "token.hpp"
#include <string>
#include <vector>
std::vector<Token> preprocess(const std::string &entrypoint);

// for testing
std::vector<Token>
preprocess_pptokens(const std::vector<PreprocessorToken> &tokens);
