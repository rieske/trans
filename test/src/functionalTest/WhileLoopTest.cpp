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
            output whileSum(5, 2);
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("10\n");
}

TEST(Compiler, whileIterationOutput) {
    SourceProgram program{R"prg(
        int whileOutput(int it) {
            while(it > 0) {
                output it;
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

    program.compile(false);

    program.runAndExpect("1", "1\n");
    program.runAndExpect("10", "10\n9\n8\n7\n6\n5\n4\n3\n2\n1\n");
    program.runAndExpect("0", "");

    // FIXME:
    //program.runAndExpect("-1", "");
    //program.runAndExpect("-10", "");
}

}

