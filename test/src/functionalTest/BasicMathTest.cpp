#include "TestFixtures.h"

namespace {

TEST(Compiler, simpleAdditionAndSubtraction) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            int first, second;
            scanf("%d %d", &first, &second);
            printf("%d ", first+second);
            printf("%d ", second+first);
            printf("%d ", first-second);
            printf("%d", second-first);
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("0 0", "0 0 0 0");
    program.runAndExpect("0 1", "1 1 -1 1");
    program.runAndExpect("1 0", "1 1 1 -1");
    program.runAndExpect("1 1", "2 2 0 0");
}

TEST(Compiler, simpleMultiplication) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            int first, second;
            int firstProduct, secondProduct;
            scanf("%d %d", &first, &second);
            firstProduct = first*second;
            secondProduct = second*first;
            printf("%d %d ", firstProduct, secondProduct);
            printf("%d ", firstProduct == secondProduct);
            printf("%d", firstProduct != secondProduct);
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("0 0", "0 0 1 0");
    program.runAndExpect("0 1", "0 0 1 0");
    program.runAndExpect("1 0", "0 0 1 0");
    program.runAndExpect("-1 0", "0 0 1 0");
    program.runAndExpect("1 1", "1 1 1 0");
    program.runAndExpect("-1 1", "-1 -1 1 0");
    program.runAndExpect("1 -1", "-1 -1 1 0");
    program.runAndExpect("-1 -1", "1 1 1 0");
    program.runAndExpect("1 2", "2 2 1 0");
    program.runAndExpect("2 1", "2 2 1 0");
    program.runAndExpect("2 2", "4 4 1 0");
}

TEST(Compiler, multiplicationLeavesBothOperandsUsable) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int a0, a1, a2, a3, a4, a5, a6, a7, a8, a9;
            int x, y, z;
            a0 = 1; a1 = 1; a2 = 1; a3 = 1; a4 = 1;
            a5 = 1; a6 = 1; a7 = 1; a8 = 1; a9 = 1;
            x = 3;
            y = 5;
            z = x * y;
            printf("%d %d %d %d", x, y, z,
                    a0 + a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3 5 15 10");
}

// FIXME: %ld - ints treated as longs for now
TEST(Compiler, simpleDivision) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            int first, second;
            scanf("%ld %ld", &first, &second);
            printf("%d", first/second);
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("0 1", "0");
    program.runAndExpect("1 1", "1");
    program.runAndExpect("2 1", "2");
    program.runAndExpect("2 2", "1");
    program.runAndExpect("4 2", "2");
    program.runAndExpect("15 3", "5");

    program.runAndExpect("2 3", "0");
    program.runAndExpect("3 2", "1");
    program.runAndExpect("5 2", "2");
}

// Signed idiv must CQO (sign-extend RAX into RDX:RAX), not zero RDX.
// Fuzzer oracle: (-1)/(-1) SIGFPE'd with xor rdx,rdx (rdx:rax became a huge positive).
TEST(Compiler, signedDivisionOfNegatives) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            int a;
            int b;
            a = -1;
            b = -1;
            printf("%d ", a / b);
            a = -20;
            b = 6;
            printf("%d ", a / b);
            a = 20;
            b = -6;
            printf("%d ", a / b);
            a = -20;
            b = -6;
            printf("%d", a / b);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 -3 -3 3");
}

// FIXME: %ld - ints treated as longs for now
TEST(Compiler, simpleModulus) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            int first, second;
            scanf("%ld %ld", &first, &second);
            printf("%d", first%second);
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("0 1", "0");
    program.runAndExpect("1 1", "0");
    program.runAndExpect("2 1", "0");
    program.runAndExpect("4 2", "0");
    program.runAndExpect("15 3", "0");

    program.runAndExpect("2 3", "2");
    program.runAndExpect("3 2", "1");
    program.runAndExpect("5 2", "1");
}

// C99 toward-zero remainder for negatives: a%b == a - (a/b)*b
TEST(Compiler, signedModuloOfNegatives) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            int a;
            int b;
            a = -20;
            b = 6;
            printf("%d ", a % b);
            a = 20;
            b = -6;
            printf("%d ", a % b);
            a = -20;
            b = -6;
            printf("%d ", a % b);
            a = -1;
            b = -1;
            printf("%d", a % b);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("-2 2 -2 0");
}

TEST(Compiler, unsignedLongDivMod) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            unsigned long a;
            a = 0x8000000000000000UL;
            printf("%lu %lu ", a / 2UL, a % 3UL);
            a = 0xffffffffffffffffUL;
            printf("%lu %lu", a / 2UL, a % 2UL);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4611686018427387904 2 9223372036854775807 1");
}

}
