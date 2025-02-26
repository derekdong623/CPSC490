#include "test_framework.hpp"

#include<string>
#include<functional>
#include<vector>
#include<iostream>

namespace testing {
    void run_tests(const std::vector<TestCase>& tests) {
        int passed = 0;
        for (const auto& test : tests) {
            bool success = test.func();
            std::cout << (success ? "[PASS] " : "[FAIL] ") << test.name << std::endl;
            passed += success;
        }
        std::cout << "Summary: " << passed << "/" << tests.size() << " tests passed." << std::endl;
    }
}