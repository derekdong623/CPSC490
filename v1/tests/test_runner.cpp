#include "test_framework.hpp"

extern std::vector<testing::TestCase> init_tests; // Declare test groups

int main() {
    std::vector<testing::TestCase> all_tests;
    all_tests.insert(all_tests.end(), init_tests.begin(), init_tests.end());
    testing::run_tests(all_tests);
    return 0;
}