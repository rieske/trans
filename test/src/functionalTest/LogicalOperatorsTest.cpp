#include "TestFixtures.h"

namespace {

TEST(Compiler, logicalNot) {
    SourceProgram program{R"prg(
        int main() {
            int a;
            input a;
            output !a;
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("0", "1\n");
    program.runAndExpect("1", "0\n");
    program.runAndExpect("2", "0\n");
    program.runAndExpect("-1", "0\n");
}

TEST(Compiler, logicalAnd) {
    SourceProgram program{R"prg(
        int main() {
            int a, b;
            input a;
            input b;
            output a && b;
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("0\n0", "0\n");
    program.runAndExpect("1\n0", "0\n");
    program.runAndExpect("0\n1", "0\n");
    program.runAndExpect("1\n1", "1\n");

    program.runAndExpect("11\n7", "1\n"); // 1011 && 0111 = 0001
}

TEST(Compiler, bitwiseAnd) {
    SourceProgram program{R"prg(
        int main() {
            int a, b;
            input a;
            input b;
            output a & b;
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("0\n0", "0\n");
    program.runAndExpect("1\n0", "0\n");
    program.runAndExpect("0\n1", "0\n");
    program.runAndExpect("1\n1", "1\n");

    program.runAndExpect("11\n7", "3\n"); // 1011 & 0111 = 0011
}

TEST(Compiler, logicalOr) {
    SourceProgram program{R"prg(
        int main() {
            int a, b;
            input a;
            input b;
            output a || b;
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("0\n0", "0\n");
    program.runAndExpect("1\n0", "1\n");
    program.runAndExpect("0\n1", "1\n");
    program.runAndExpect("1\n1", "1\n");

    program.runAndExpect("11\n5", "1\n"); // 1011 || 0101 = 0001
}

TEST(Compiler, bitwiseOr) {
    SourceProgram program{R"prg(
        int main() {
            int a, b;
            input a;
            input b;
            output a | b;
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("0\n0", "0\n");
    program.runAndExpect("1\n0", "1\n");
    program.runAndExpect("0\n1", "1\n");
    program.runAndExpect("1\n1", "1\n");

    program.runAndExpect("11\n5", "15\n"); // 1011 | 0101 = 1111
}

TEST(Compiler, bitwiseXor) {
    SourceProgram program{R"prg(
        int main() {
            int a, b;
            input a;
            input b;
            output a ^ b;
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("0\n0", "0\n");
    program.runAndExpect("1\n0", "1\n");
    program.runAndExpect("0\n1", "1\n");
    program.runAndExpect("1\n1", "0\n");

    program.runAndExpect("11\n5", "14\n"); // 1011 ^ 0101 = 1110
}

}
