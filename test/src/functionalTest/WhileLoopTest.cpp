#include "TestFixtures.h"

namespace {

TEST(Compiler, compilesWhileLoopSumProgram) {
    SourceProgram program{R"prg(
        int whileSum(int it, int add) {
            int result = 0;
            while(it > 0) {
                result = result + add;
                it = it - 1;
            }
            return result;
        }

        int main() {
            printf("%d", whileSum(5, 2));
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("10");
}

TEST(Compiler, whileIterationOutput) {
    SourceProgram program{R"prg(
        int whileOutput(int it) {
            while(it > 0) {
                printf("%d ", it);
                it = it - 1;
            }
            return 0;
        }

        int main() {
            int it;
            input it;
            whileOutput(it);
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("1", "1 ");
    program.runAndExpect("10", "10 9 8 7 6 5 4 3 2 1 ");
    program.runAndExpect("0", "");

    program.runAndExpect("-1", "");
    program.runAndExpect("-10", "");
}

TEST(Compiler, whileIterationOutputConstNegative) {
    SourceProgram program{R"prg(
        int whileOutput(int it) {
            while(it > 0) {
                printf("%d ", it);
                it = it - 1;
            }
            return 0;
        }

        int main() {
            whileOutput(-1);
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("");
}

}

