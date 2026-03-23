#pragma once

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/ioctl.h>
#include <unistd.h>
#endif

namespace core_utils {

struct TerminalSize {
  int width;
  int height;
  bool success;
};

inline TerminalSize get_terminal_size() {
  TerminalSize size = {0, 0, false};

#ifdef _WIN32
  CONSOLE_SCREEN_BUFFER_INFO csbi;
  if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
    size.width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    size.height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    size.success = true;
  } else {
    size.success = false;
  }
#else
  struct winsize w;
  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) != -1) {
    size.width = w.ws_col;
    size.height = w.ws_row;
    size.success = true;
  } else {
    size.success = false;
  }
#endif
  return size;
}
}; // namespace core_utils
