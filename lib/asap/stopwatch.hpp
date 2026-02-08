#ifndef _ASAP_STOPWATCH
#define _ASAP_STOPWATCH

#include <chrono>
class StopWatch {
private:
  std::chrono::high_resolution_clock::time_point start_ts;

public:
  StopWatch() : start_ts(std::chrono::high_resolution_clock::now()) {}
  void reset() { this->start_ts = std::chrono::high_resolution_clock::now(); }
  long stop_ms() {
    std::chrono::time_point stop = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(stop -
                                                                 this->start_ts)
        .count();
  }
  long stop_micro() {
    std::chrono::time_point stop = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::microseconds>(stop -
                                                                 this->start_ts)
        .count();
  }
};
#endif
