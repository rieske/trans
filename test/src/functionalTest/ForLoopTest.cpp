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
            output forSum(5, 2);
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("10\n");
}

TEST(Compiler, forLoopIterationOutput) {
    SourceProgram program{R"prg(
        int iterationOutput(int n) {
            int i;
            for (i = 0; i <= n; i++) {
                output i;
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

    program.runAndExpect("10", "0\n1\n2\n3\n4\n5\n6\n7\n8\n9\n10\n");
    program.runAndExpect("5", "0\n1\n2\n3\n4\n5\n");
    program.runAndExpect("1", "0\n1\n");
    program.runAndExpect("0", "0\n");
    program.runAndExpect("-1", "");
}

TEST(Compiler, forLoopLessThan) {
    SourceProgram program{R"prg(
        int iterationOutput(int n) {
            int i;
            for (i = 0; i < n; i++) {
                output i;
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

    program.runAndExpect("10", "0\n1\n2\n3\n4\n5\n6\n7\n8\n9\n");
    program.runAndExpect("5", "0\n1\n2\n3\n4\n");
    program.runAndExpect("1", "0\n");
    program.runAndExpect("0", "");
    program.runAndExpect("-1", "");
}

}
