#pragma once

#include "location.hpp"
#include <stdexcept>
namespace {
inline std::runtime_error __compile_error(LocationRef location,
                                          std::string message) {
  std::string loc = location_to_string(location);
  return std::runtime_error("ERROR [at " + loc + "]: " + message + "\n");
}
inline std::runtime_error __compile_error_debug(LocationRef location,
                                                std::string message,
                                                int cppline,
                                                std::string cppfile) {
  std::string loc = location_to_string(location);
  return std::runtime_error("<" + cppfile + "|" + std::to_string(cppline) +
                            "> ERROR [at " + loc + "]: " + message + "\n");
}
} // namespace

#ifndef PRODUCTION
#define compile_error(loc, msg)                                                \
  __compile_error_debug(loc, msg, __LINE__, __FILE__)
#else
#define compile_error(loc, msg) __compile_error(loc, msg)
#endif
