#include "TestFixtures.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include <iostream>

namespace {

class AssemblyBackendListener : public ::testing::EmptyTestEventListener {
public:
    void OnTestProgramStart(const ::testing::UnitTest& /*unit_test*/) override {
        std::cout << "[==========] Assembly backend: " << functionalTestDialectTag()
                  << "  opt=" << functionalTestOptFlag() << "\n";
    }

    void OnTestStart(const ::testing::TestInfo& test_info) override {
        std::cout << "[ MATRIX   ] " << functionalTestMatrixTag()
                  << "  " << test_info.test_suite_name() << "." << test_info.name() << "\n";
    }

    void OnTestEnd(const ::testing::TestInfo& test_info) override {
        if (test_info.result()->Failed()) {
            std::cout << "[  FAILED  ] matrix=" << functionalTestMatrixTag()
                      << "  " << test_info.test_suite_name() << "." << test_info.name() << "\n";
        }
    }

    void OnTestProgramEnd(const ::testing::UnitTest& unit_test) override {
        if (unit_test.failed_test_count() > 0) {
            std::cout << "[==========] Failures above are for matrix: "
                      << functionalTestMatrixTag() << "\n";
        }
    }
};

} // namespace

int main(int argc, char** argv) {
    ::testing::InitGoogleMock(&argc, argv);
    ::testing::UnitTest::GetInstance()->listeners().Append(new AssemblyBackendListener);
    return RUN_ALL_TESTS();
}
