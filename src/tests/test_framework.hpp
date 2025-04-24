#pragma once

#include <functional>
#include <string>
#include <vector>

#define TIME_EXPR(label, code)                                                                     \
  do {                                                                                             \
    auto start = std::chrono::high_resolution_clock::now();                                        \
    code auto end = std::chrono::high_resolution_clock::now();                                     \
    std::chrono::duration<double, std::milli> elapsed = end - start;                               \
    std::cout << label << ": " << elapsed.count() << " ms\n";                                      \
  } while (0)
#define TIME_EXPR_RET(label, expr)                                                                 \
  [&]() {                                                                                          \
    auto start = std::chrono::high_resolution_clock::now();                                        \
    auto result = (expr);                                                                          \
    auto end = std::chrono::high_resolution_clock::now();                                          \
    std::chrono::duration<double, std::milli> elapsed = end - start;                               \
    std::cout << label << ": " << elapsed.count() << " ms\n";                                      \
    return result;                                                                                 \
  }()
#define TIME_EXPR_TIMING(code)                                                                     \
  [&]() {                                                                                          \
    auto start = std::chrono::high_resolution_clock::now();                                        \
    code auto end = std::chrono::high_resolution_clock::now();                                     \
    std::chrono::duration<double, std::milli> elapsed = end - start;                               \
    return elapsed.count();                                                                        \
  }()

namespace testing {
struct TestCase {
  std::string name;
  std::function<bool()> func;
};
void run_tests(const std::vector<TestCase> &tests);
} // namespace testing