#include "TestFixtures.h"

#include <fstream>

namespace {

TEST(Compiler, compilesSwapProgram) {
    // FIXME: segfaults with printf!
    SourceProgram program{R"prg(
        int swap(int *x, int *y) {
            int temp;
            printf("%d\n%d\n", *x, *y);
            temp = *x;
            *x = *y;
            *y = temp;
            // FIXME: segfaults!, maybe clear rax before calling
            // printf("%d\n%d\n", *x, *y);
            output *x;
            output *y;
            return 0;
        }

        int main() {
            int a = 0;
            int b = 1;
            printf("%d\n%d\n", a, b);
            printf("%d\n%d\n", &a, &b);
            swap (&a, &b);
            printf("%d\n%d\n", a, b);
            printf("%d\n%d\n", &a, &b);
            return 0;
        }
    )prg"};
    program.compile();
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
