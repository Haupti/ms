#ifndef __ASAP_UTIL
#define __ASAP_UTIL

#include <iostream>
#include <vector>
namespace ANSI {
static std::string BLACK = "\x1b[30m";
static std::string RED = "\x1b[31m";
static std::string GREEN = "\x1b[32m";
static std::string YELLOW = "\x1b[33m";
static std::string BLUE = "\x1b[34m";
static std::string MAGENTA = "\x1b[35m";
static std::string CYAN = "\x1b[36m";
static std::string WHITE = "\x1b[37m";

static std::string BLACK_BG = "\x1b[40m";
static std::string RED_BG = "\x1b[41m";
static std::string GREEN_BG = "\x1b[42m";
static std::string YELLOW_BG = "\x1b[43m";
static std::string BLUE_BG = "\x1b[44m";
static std::string MAGENTA_BG = "\x1b[45m";
static std::string CYAN_BG = "\x1b[46m";
static std::string WHITE_BG = "\x1b[47m";

static std::string BRIGHT_BLACK = "\x1b[90m";
static std::string BRIGHT_RED = "\x1b[91m";
static std::string BRIGHT_GREEN = "\x1b[92m";
static std::string BRIGHT_YELLOW = "\x1b[93m";
static std::string BRIGHT_BLUE = "\x1b[94m";
static std::string BRIGHT_MAGENTA = "\x1b[95m";
static std::string BRIGHT_CYAN = "\x1b[96m";
static std::string BRIGHT_WHITE = "\x1b[97m";

static std::string BRIGHT_BLACK_BG = "\x1b[100m";
static std::string BRIGHT_RED_BG = "\x1b[101m";
static std::string BRIGHT_GREEN_BG = "\x1b[102m";
static std::string BRIGHT_YELLOW_BG = "\x1b[103m";
static std::string BRIGHT_BLUE_BG = "\x1b[104m";
static std::string BRIGHT_MAGENTA_BG = "\x1b[105m";
static std::string BRIGHT_CYAN_BG = "\x1b[106m";
static std::string BRIGHT_WHITE_BG = "\x1b[107m";

static std::string BRIGHT_GRAY = "\x1b[37m";
static std::string CR = "\r";

static std::string CURSOR_UP = "\x1b[A";
static std::string CURSOR_DOWN = "\x1b[B";
static std::string CURSOR_FORWARD = "\x1b[C";
static std::string CURSOR_BACK = "\x1b[D";
static std::string CURSOR_NEXTL = "\x1b[E";
static std::string CURSOR_PREVL = "\x1b[F";

static std::string ERASE_LINE = "\x1b[2K";
static std::string ERASE_SCREEN = "\x1b[2J";

static std::string BOLD = "\x1b[1m";
static std::string ITALIC = "\x1b[3m";
static std::string UNDERLINE = "\x1b[4m";
static std::string BLINK = "\x1b[5m";

static std::string RESET = "\x1b[0m";
}; // namespace ANSI
// logging
inline void println(const std::string &msg) { std::cout << msg << "\n"; }

inline void print(const std::string &msg) { std::cout << msg; }
inline void warn(const std::string &msg) {
  println(ANSI::BRIGHT_YELLOW + msg + ANSI::RESET);
}
inline void info(const std::string &msg) {
  println(ANSI::BRIGHT_GRAY + msg + ANSI::RESET);
}

// error and user error
inline void fail [[noreturn]] (const std::string &msg) {
  println(ANSI::RED + msg + ANSI::RESET);
  exit(1);
}
#define panic(msg)                                                             \
  throw std::runtime_error("ERROR in " + std::string(__FILE__) + "/" +         \
                           std::to_string(__LINE__) + ": " + msg + "\n");
inline void overwrite_println(const std::string &msg) {
  println(ANSI::CURSOR_UP + ANSI::ERASE_LINE + ANSI::CR + ANSI::BRIGHT_GRAY +
          msg + ANSI::RESET);
}
// file utilities
void write_file(const std::string &filepath, const std::string &content);
std::string read_file(const std::string &filepath);

// string utilities
bool ends_with(const std::string &str, const std::string &suffix);
bool starts_with(const std::string &str, const std::string &prefix);
std::string join(const std::vector<std::string> &list, std::string delim);

#endif
