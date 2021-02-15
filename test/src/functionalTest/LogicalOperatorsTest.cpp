#include "TestFixtures.h"

namespace {

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
}

}
