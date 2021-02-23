#include "TestFixtures.h"

namespace {

TEST(Compiler, compilesForLoopSumProgram) {
    SourceProgram program{R"prg(
        int forSum(int it, int add) {
            int i;
            int result = 0;
            for (i = 0; i < it; i++) {
                result = result + add;
            }
            return result;
        }

        int main() {
            printf("%d", forSum(5, 2));
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("10");
}

TEST(Compiler, forLoopIterationOutput) {
    SourceProgram program{R"prg(
        int iterationOutput(int n) {
            int i;
            for (i = 0; i <= n; i++) {
                printf("%d ", i);
            }
            return 0;
        }

        int main() {
            int n;
            input n;
            iterationOutput(n);
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("10", "0 1 2 3 4 5 6 7 8 9 10 ");
    program.runAndExpect("5", "0 1 2 3 4 5 ");
    program.runAndExpect("1", "0 1 ");
    program.runAndExpect("0", "0 ");
    program.runAndExpect("-1", "");
}

TEST(Compiler, forLoopLessThan) {
    SourceProgram program{R"prg(
        int iterationOutput(int n) {
            int i;
            for (i = 0; i < n; i++) {
                printf("%d ", i);
            }
            return 0;
        }

        int main() {
            int n;
            input n;
            iterationOutput(n);
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("10", "0 1 2 3 4 5 6 7 8 9 ");
    program.runAndExpect("5", "0 1 2 3 4 ");
    program.runAndExpect("1", "0 ");
    program.runAndExpect("0", "");
    program.runAndExpect("-1", "");
}

}
