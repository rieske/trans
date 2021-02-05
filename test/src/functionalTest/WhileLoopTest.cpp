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

}

