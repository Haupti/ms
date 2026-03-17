#pragma once

#include "preprocessor_token.hpp"
#include <vector>

std::vector<PreprocessorToken>
preprocessor_tokenize(const std::string *const filename,
                      const std::string &code);
