#pragma once

#include <functional>
#include <string>
#include <vector>

namespace testing {
struct TestCase {
  std::string name;
  std::function<bool()> func;
};
void run_tests(const std::vector<TestCase> &tests);
} // namespace testing