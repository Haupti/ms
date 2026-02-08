#include "util.hpp"
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>

void write_file(const std::string &filepath, const std::string &content) {
  std::ofstream f(filepath);
  if (!f.good()) {
    panic("failed to write file '" + filepath + "'");
  }
  f << content;
}
std::string read_file(const std::string &filepath) {
  std::ifstream f(filepath);
  if (!f.good()) {
    panic("failed to read file '" + filepath + "'");
  }
  uint64_t size = std::filesystem::file_size(filepath);
  std::string buffer(size, '\0');
  f.read(buffer.data(), size);
  return buffer;
}

bool ends_with(const std::string &str, const std::string &suffix) {
  if (str.size() < suffix.size()) {
    return false;
  }
  return str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

bool starts_with(const std::string &str, const std::string &prefix) {
  if (str.size() < prefix.size()) {
    return false;
  }
  return str.compare(0, prefix.size(), prefix) == 0;
}

std::string join(const std::vector<std::string> &list, std::string delim) {
  std::string result;
  for (size_t i = 0; i < list.size(); i++) {
    result += list[i];
    if (i != (list.size() - 1)) {
      result += delim;
    }
  }
  return result;
}
