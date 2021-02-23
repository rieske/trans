#include "TestFixtures.h"

namespace {

TEST(Compiler, logicalNot) {
    SourceProgram program{R"prg(
        int main() {
            int a;
            input a;
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
            input a;
            input b;
            printf("%d", a && b);
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("0\n0", "0");
    program.runAndExpect("1\n0", "0");
    program.runAndExpect("0\n1", "0");
    program.runAndExpect("1\n1", "1");

    program.runAndExpect("11\n7", "1"); // 1011 && 0111 = 0001
}

TEST(Compiler, bitwiseAnd) {
    SourceProgram program{R"prg(
        int main() {
            int a, b;
            input a;
            input b;
            printf("%d", a & b);
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("0\n0", "0");
    program.runAndExpect("1\n0", "0");
    program.runAndExpect("0\n1", "0");
    program.runAndExpect("1\n1", "1");

    program.runAndExpect("11\n7", "3"); // 1011 & 0111 = 0011
}

TEST(Compiler, logicalOr) {
    SourceProgram program{R"prg(
        int main() {
            int a, b;
            input a;
            input b;
            printf("%d", a || b);
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("0\n0", "0");
    program.runAndExpect("1\n0", "1");
    program.runAndExpect("0\n1", "1");
    program.runAndExpect("1\n1", "1");

    program.runAndExpect("11\n5", "1"); // 1011 || 0101 = 0001
}

TEST(Compiler, bitwiseOr) {
    SourceProgram program{R"prg(
        int main() {
            int a, b;
            input a;
            input b;
            printf("%d", a | b);
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("0\n0", "0");
    program.runAndExpect("1\n0", "1");
    program.runAndExpect("0\n1", "1");
    program.runAndExpect("1\n1", "1");

    program.runAndExpect("11\n5", "15"); // 1011 | 0101 = 1111
}

TEST(Compiler, bitwiseXor) {
    SourceProgram program{R"prg(
        int main() {
            int a, b;
            input a;
            input b;
            printf("%d", a ^ b);
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("0\n0", "0");
    program.runAndExpect("1\n0", "1");
    program.runAndExpect("0\n1", "1");
    program.runAndExpect("1\n1", "0");

    program.runAndExpect("11\n5", "14"); // 1011 ^ 0101 = 1110
}

}
