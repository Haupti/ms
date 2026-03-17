#pragma once

#include "filename.hpp"
#include <string>

std::string find_sourcecode(Filename filename);
void cache_sourcecode(Filename filename, const std::string &sourcecode);
