#include "test_framework.hpp"

// Declare test groups
extern std::vector<testing::TestCase> init_tests;
extern std::vector<testing::TestCase> battle_tests;

int main() {
    std::vector<testing::TestCase> all_tests;
    all_tests.insert(all_tests.end(), init_tests.begin(), init_tests.end());
    all_tests.insert(all_tests.end(), battle_tests.begin(), battle_tests.end());
    testing::run_tests(all_tests);
    return 0;
}