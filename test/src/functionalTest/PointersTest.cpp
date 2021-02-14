#include "TestFixtures.h"

#include <fstream>

namespace {

TEST(Compiler, compilesSwapProgram) {
    SourceProgram program{R"prg(
        int swap(int *x, int *y) {
            int temp;
            output *x;
            output *y;
            temp = *x;
            *x = *y;
            *y = temp;
            output *x;
            output *y;
            return 0;
        }

        int main() {
            int a = 0;
            int b = 1;
            output a;
            output b;
            output &a;
            output &b;
            swap (&a, &b);
            output a;
            output b;
            output &a;
            output &b;
            return 0;
        }
    )prg"};
    program.compile(true);
    program.run();

    std::ifstream expectedOutputStream{program.getOutputFilePath()};
    std::string outputLine;
    expectedOutputStream >> outputLine;
    EXPECT_THAT(outputLine, Eq("0"));
    expectedOutputStream >> outputLine;
    EXPECT_THAT(outputLine, Eq("1"));
    std::string firstAddressBefore;
    expectedOutputStream >> firstAddressBefore;
    std::string secondAddressBefore;
    expectedOutputStream >> secondAddressBefore;
    expectedOutputStream >> outputLine;
    EXPECT_THAT(outputLine, Eq("0"));
    expectedOutputStream >> outputLine;
    EXPECT_THAT(outputLine, Eq("1"));
    expectedOutputStream >> outputLine;
    EXPECT_THAT(outputLine, Eq("1"));
    expectedOutputStream >> outputLine;
    EXPECT_THAT(outputLine, Eq("0"));
    expectedOutputStream >> outputLine;
    EXPECT_THAT(outputLine, Eq("1"));
    expectedOutputStream >> outputLine;
    EXPECT_THAT(outputLine, Eq("0"));
    std::string firstAddressAfter;
    expectedOutputStream >> firstAddressAfter;
    std::string secondAddressAfter;
    expectedOutputStream >> secondAddressAfter;
    EXPECT_THAT(firstAddressBefore, Not(Eq(secondAddressBefore)));
    EXPECT_THAT(firstAddressBefore, Eq(firstAddressAfter));
    EXPECT_THAT(secondAddressBefore, Eq(secondAddressAfter));
}

}
