#pragma once

#include "location.hpp"
#include <stdexcept>
namespace {
inline std::runtime_error __msl_runtime_error(LocationRef location,
                                              std::string message) {
  return std::runtime_error("ERROR [at " + location_to_string(location) +
                            "]: " + message + "\n");
}
inline std::runtime_error __msl_runtime_error_debug(LocationRef location,
                                                    std::string message,
                                                    int cppline,
                                                    std::string cppfile) {
  return std::runtime_error("<" + cppfile + "|" + std::to_string(cppline) +
                            "> ERROR [at " + location_to_string(location) +
                            "]: " + message + "\n");
}
} // namespace

#ifndef PRODUCTION
#define msl_runtime_error(loc, msg)                                            \
  __msl_runtime_error_debug(loc, msg, __LINE__, __FILE__)
#else
#define msl_runtime_error(loc, msg) __msl_runtime_error(loc, msg)
#endif
