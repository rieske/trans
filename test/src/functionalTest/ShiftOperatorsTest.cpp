#include "TestFixtures.h"

namespace {

TEST(Compiler, shiftLeft) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
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
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
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
    // Signed arithmetic shift (SAR): high bits filled with sign bit.
    program.runAndExpect("-8", "-4 -2 -1");
    program.runAndExpect("-1", "-1 -1 -1");
}

TEST(Compiler, unsignedShiftRightIsLogical) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            unsigned u;
            unsigned long ul;
            u = 0x80000000u;
            ul = 0x8000000000000000UL;
            printf("%u %u ", u >> 1, u >> 2);
            printf("%lu %lu", ul >> 1, ul >> 63);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1073741824 536870912 4611686018427387904 1");
}

} // namespace

