#include "TestFixtures.h"

namespace {

TEST(Compiler, shiftLeft) {
    SourceProgram program{R"prg(
        int main() {
            int a;
            scanf("%ld", &a);
            printf("%d ", a << 1);
            printf("%d ", a << 2);
            printf("%d", a << 3);
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("0", "0 0 0");
    program.runAndExpect("1", "2 4 8");
    program.runAndExpect("2", "4 8 16");
}

TEST(Compiler, shiftRight) {
    SourceProgram program{R"prg(
        int main() {
            int a;
            scanf("%ld", &a);
            printf("%d ", a >> 1);
            printf("%d ", a >> 2);
            printf("%d", a >> 3);
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("0", "0 0 0");
    program.runAndExpect("8", "4 2 1");
    program.runAndExpect("16", "8 4 2");
}

} // namespace

