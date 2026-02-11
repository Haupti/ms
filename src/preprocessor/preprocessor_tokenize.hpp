#pragma once

#include "preprocessor_token.hpp"
#include <vector>

std::vector<PreprocessorToken>
preprocessor_tokenize(const std::filesystem::path &filename, const std::string &code);
