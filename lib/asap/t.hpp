#ifndef _ASAP_T
#define _ASAP_T

#include "stopwatch.hpp"
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>

class TestCaseFailure : public std::runtime_error {
public:
  TestCaseFailure(const std::string &message) : std::runtime_error(message) {}

  const char *what() const noexcept override { return runtime_error::what(); }
};

class T {
private:
  const std::string ansi_fg_red = "[31m";
  const std::string ansi_fg_green = "[32m";
  const std::string ansi_reset = "[0m";
  std::vector<std::string> failures_testnames;
  std::vector<std::string> failures_messages;
  std::vector<std::string> successes_testnames;
  std::string suitname;

public:
  T(const std::string &suitname_param) {
    failures_testnames = {};
    failures_messages = {};
    successes_testnames = {};
    suitname = suitname_param;
  };

  void test(const std::string &testname,
            const std::function<void(T *)> &testcase) {
    StopWatch sw = StopWatch();
    try {
      testcase(this);
    } catch (TestCaseFailure e) {
      long duration_micro = sw.stop_micro();
      std::cout << ansi_fg_red + "FAILURE [" + suitname + "] of '" + testname +
                       "': " + e.what() + ansi_reset + " (" +
                       std::to_string(duration_micro) + "μs)\n";
      return;
    } catch (std::runtime_error e) {
      long duration_micro = sw.stop_micro();
      std::cout << ansi_fg_red + "FAILURE [" + suitname + "] of '" + testname +
                       "' failed with runtime error: '" + e.what() + "'" +
                       ansi_reset + " (" + std::to_string(duration_micro) +
                       "μs)\n";
      return;
    }
    long duration_micro = sw.stop_micro();
    std::cout << ansi_fg_green + "SUCCESS [" + suitname + "]: " + testname +
                     ansi_reset + " (" + std::to_string(duration_micro) +
                     "μs)\n";
  }

  void assert(bool cond) {
    if (!cond) {
      throw TestCaseFailure("test on condition failed");
    }
  }
  void assert_str_eq(const std::string &actual, const std::string &expected) {
    if (actual != expected) {
      throw TestCaseFailure("'" + actual + "'is not equal to expected '" + expected + "'");
    }
  }
  void fail(const std::string &msg) { throw TestCaseFailure(msg); }
  void success() {}
  void assert_throws_with_message_contains(const std::function<void()> &fn,
                                           const std::string &error_msg) {
    try {
      fn();
    } catch (std::runtime_error e) {
      if (std::string(e.what()).find(error_msg) == std::string::npos) {
        this->fail("expected runtime_error message '" + std::string(e.what()) +
                   "' to contain '" + error_msg + "'");
      }
      return;
    }
    this->fail("expected a runtime_error");
  }
  void assert_throws(const std::function<void()> &fn) {
    try {
      fn();
    } catch (std::runtime_error e) {
      this->success();
      return;
    }
    this->fail("expected a runtime_error");
  };
};
#endif
