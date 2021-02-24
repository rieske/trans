#include "TestFixtures.h"

namespace {

TEST(Compiler, logicalNot) {
    SourceProgram program{R"prg(
        int main() {
            int a;
            scanf("%ld", &a);
            printf("%d", !a);
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("0", "1");
    program.runAndExpect("1", "0");
    program.runAndExpect("2", "0");
    program.runAndExpect("-1", "0");
}

TEST(Compiler, logicalAnd) {
    SourceProgram program{R"prg(
        int main() {
            int a, b;
            scanf("%ld %ld", &a, &b);
            printf("%d", a && b);
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("0 0", "0");
    program.runAndExpect("1 0", "0");
    program.runAndExpect("0 1", "0");
    program.runAndExpect("1 1", "1");

    program.runAndExpect("11 7", "1"); // 1011 && 0111 = 0001
}

TEST(Compiler, bitwiseAnd) {
    SourceProgram program{R"prg(
        int main() {
            int a, b;
            scanf("%ld %ld", &a, &b);
            printf("%d", a & b);
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("0 0", "0");
    program.runAndExpect("1 0", "0");
    program.runAndExpect("0 1", "0");
    program.runAndExpect("1 1", "1");

    program.runAndExpect("11 7", "3"); // 1011 & 0111 = 0011
}

TEST(Compiler, logicalOr) {
    SourceProgram program{R"prg(
        int main() {
            int a, b;
            scanf("%ld %ld", &a, &b);
            printf("%d", a || b);
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("0 0", "0");
    program.runAndExpect("1 0", "1");
    program.runAndExpect("0 1", "1");
    program.runAndExpect("1 1", "1");

    program.runAndExpect("11 5", "1"); // 1011 || 0101 = 0001
}

TEST(Compiler, bitwiseOr) {
    SourceProgram program{R"prg(
        int main() {
            int a, b;
            scanf("%ld %ld", &a, &b);
            printf("%d", a | b);
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("0 0", "0");
    program.runAndExpect("1 0", "1");
    program.runAndExpect("0 1", "1");
    program.runAndExpect("1 1", "1");

    program.runAndExpect("11 5", "15"); // 1011 | 0101 = 1111
}

TEST(Compiler, bitwiseXor) {
    SourceProgram program{R"prg(
        int main() {
            int a, b;
            scanf("%ld %ld", &a, &b);
            printf("%d", a ^ b);
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("0 0", "0");
    program.runAndExpect("1 0", "1");
    program.runAndExpect("0 1", "1");
    program.runAndExpect("1 1", "0");

    program.runAndExpect("11 5", "14"); // 1011 ^ 0101 = 1110
}

}
