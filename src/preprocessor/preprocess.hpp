#pragma once

#include "preprocessor_token.hpp"
#include "token.hpp"
#include <string>
#include <unordered_set>
#include <vector>

struct IncludedModules {
  // all realtive to entrypoint
  std::unordered_set<std::string> absolute_filepaths;
};

std::vector<Token> preprocess(const std::string &entrypoint);

// for testing
std::vector<Token>
preprocess_pptokens(const std::string &absolute_current_path,
                    IncludedModules *includes,
                    const std::vector<PreprocessorToken> &tokens);
