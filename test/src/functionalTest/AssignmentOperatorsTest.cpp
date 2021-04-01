#include "TestFixtures.h"

namespace {

TEST(Compiler, assign) {
    SourceProgram program{R"prg(
        int main() {
            int a, b;
            scanf("%ld", &a);
            b = a;
            printf("%d %d", a, b);
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("0", "0 0");
    program.runAndExpect("1", "1 1");
    program.runAndExpect("2", "2 2");
}

TEST(Compiler, addAssign) {
    SourceProgram program{R"prg(
        int main() {
            int a, b;
            scanf("%ld %ld", &a, &b);
            b += a;
            printf("%d %d", a, b);
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("0 0", "0 0");
    program.runAndExpect("0 1", "0 1");
    program.runAndExpect("1 0", "1 1");
    program.runAndExpect("1 1", "1 2");
    program.runAndExpect("1 2", "1 3");
    program.runAndExpect("2 2", "2 4");
}

TEST(Compiler, subAssign) {
    SourceProgram program{R"prg(
        int main() {
            int a, b;
            scanf("%ld %ld", &a, &b);
            b -= a;
            printf("%d %d", a, b);
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("0 0", "0 0");
    program.runAndExpect("0 1", "0 1");
    program.runAndExpect("1 0", "1 -1");
    program.runAndExpect("1 1", "1 0");
    program.runAndExpect("1 2", "1 1");
    program.runAndExpect("2 2", "2 0");
}

TEST(Compiler, mulAssign) {
    SourceProgram program{R"prg(
        int main() {
            int a, b;
            scanf("%ld %ld", &a, &b);
            b *= a;
            printf("%d %d", a, b);
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("0 0", "0 0");
    program.runAndExpect("0 1", "0 0");
    program.runAndExpect("1 0", "1 0");
    program.runAndExpect("1 1", "1 1");
    program.runAndExpect("1 2", "1 2");
    program.runAndExpect("2 2", "2 4");
}

TEST(Compiler, divAssign) {
    SourceProgram program{R"prg(
        int main() {
            int a, b;
            scanf("%ld %ld", &a, &b);
            b /= a;
            printf("%d %d", a, b);
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("1 0", "1 0");
    program.runAndExpect("1 1", "1 1");
    program.runAndExpect("1 2", "1 2");
    program.runAndExpect("2 2", "2 1");
    program.runAndExpect("2 3", "2 1");
    program.runAndExpect("3 2", "3 0");
}

TEST(Compiler, modAssign) {
    SourceProgram program{R"prg(
        int main() {
            int a, b;
            scanf("%ld %ld", &a, &b);
            b %= a;
            printf("%d %d", a, b);
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("1 0", "1 0");
    program.runAndExpect("1 1", "1 0");
    program.runAndExpect("1 2", "1 0");
    program.runAndExpect("2 2", "2 0");
    program.runAndExpect("2 3", "2 1");
    program.runAndExpect("3 2", "3 2");
}

TEST(Compiler, andAssign) {
    SourceProgram program{R"prg(
        int main() {
            int a, b;
            scanf("%ld %ld", &a, &b);
            b &= a;
            printf("%d %d", a, b);
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("0 0", "0 0");
    program.runAndExpect("1 0", "1 0");
    program.runAndExpect("0 1", "0 0");
    program.runAndExpect("1 1", "1 1");

    program.runAndExpect("11 7", "11 3"); // 1011 & 0111 = 0011
}

TEST(Compiler, orAssign) {
    SourceProgram program{R"prg(
        int main() {
            int a, b;
            scanf("%ld %ld", &a, &b);
            b |= a;
            printf("%d %d", a, b);
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("0 0", "0 0");
    program.runAndExpect("1 0", "1 1");
    program.runAndExpect("0 1", "0 1");
    program.runAndExpect("1 1", "1 1");

    program.runAndExpect("11 5", "11 15"); // 1011 | 0101 = 1111
}

TEST(Compiler, xorAssign) {
    SourceProgram program{R"prg(
        int main() {
            int a, b;
            scanf("%ld %ld", &a, &b);
            b ^= a;
            printf("%d %d", a, b);
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("0 0", "0 0");
    program.runAndExpect("1 0", "1 1");
    program.runAndExpect("0 1", "0 1");
    program.runAndExpect("1 1", "1 0");

    program.runAndExpect("11 5", "11 14"); // 1011 ^ 0101 = 1110
}

TEST(Compiler, shiftLeftAssign) {
    SourceProgram program{R"prg(
        int main() {
            int a, b;
            scanf("%ld", &a);
            b = a;
            b <<= 1;
            printf("%d ", b);
            b = a;
            b <<= 2;
            printf("%d ", b);
            b = a;
            b <<= 3;
            printf("%d", b);
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("0", "0 0 0");
    program.runAndExpect("1", "2 4 8");
    program.runAndExpect("2", "4 8 16");
}

TEST(Compiler, shiftRightAssign) {
    SourceProgram program{R"prg(
        int main() {
            int a, b;
            scanf("%ld", &a);
            b = a;
            b >>= 1;
            printf("%d ", b);
            b = a;
            b >>= 2;
            printf("%d ", b);
            b = a;
            b >>= 3;
            printf("%d", b);
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("0", "0 0 0");
    program.runAndExpect("8", "4 2 1");
    program.runAndExpect("16", "8 4 2");
}

} // namespace

