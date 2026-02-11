#pragma once

#include "preprocessor_token.hpp"
#include "token.hpp"
#include <string>
#include <unordered_set>
#include <vector>

struct IncludedModules {
  std::unordered_set<std::filesystem::path>
      absolute_filepaths; // all realtive to entrypoint
};

std::vector<Token> preprocess(const std::string &entrypoint);

// for testing
std::vector<Token>
preprocess_pptokens(const std::filesystem::path &absolute_current_path,
                    IncludedModules *includes,
                    const std::vector<PreprocessorToken> &tokens);
