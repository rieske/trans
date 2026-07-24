#include "TestFixtures.h"

namespace {

TEST(Compiler, doWhileRunsAtLeastOnce) {
    SourceProgram program{R"prg(
        int main() {
            int n;
            n = 0;
            do {
                printf("%d", 7);
                n = n + 1;
            } while (0);
            printf("%d", n);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("71");
}

TEST(Compiler, doWhileMultipleIterations) {
    SourceProgram program{R"prg(
        int main() {
            int n;
            n = 0;
            do {
                n = n + 1;
            } while (n < 3);
            printf("%d", n);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3");
}

TEST(Compiler, doWhileBreak) {
    SourceProgram program{R"prg(
        int main() {
            int n;
            n = 0;
            do {
                n = n + 1;
                if (n == 2) {
                    break;
                }
            } while (1);
            printf("%d", n);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("2");
}

TEST(Compiler, doWhileContinue) {
    SourceProgram program{R"prg(
        int main() {
            int n;
            int sum;
            n = 0;
            sum = 0;
            do {
                n = n + 1;
                if (n == 2) {
                    continue;
                }
                sum = sum + n;
            } while (n < 4);
            printf("%d", sum);
            return 0;
        }
    )prg"};
    program.compile();
    // n=1 sum=1; n=2 continue; n=3 sum=4; n=4 sum=8
    program.runAndExpect("8");
}

} // namespace
