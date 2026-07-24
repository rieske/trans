#include "TestFixtures.h"

namespace {

TEST(Compiler, unaryMinusOnVariable) {
    SourceProgram program{R"prg(
        int main() {
            int a;
            scanf("%ld", &a);
            printf("%d %d", -a, -(-a));
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("0", "0 0");
    program.runAndExpect("1", "-1 1");
    program.runAndExpect("-2", "2 -2");
}

TEST(Compiler, addressOfAndDereference) {
    SourceProgram program{R"prg(
        int main() {
            int a;
            int* p;
            scanf("%ld", &a);
            p = &a;
            printf("%d %d", *p, a);
            *p = 7;
            printf(" %d %d", *p, a);
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("3", "3 3 7 7");
    program.runAndExpect("0", "0 0 7 7");
}

TEST(Compiler, logicalNotOnVariable) {
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

TEST(Compiler, bitwiseNotOnVariable) {
    SourceProgram program{R"prg(
        int main() {
            int a;
            scanf("%ld", &a);
            printf("%d %d", ~a, ~~a);
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("0", "-1 0");
    program.runAndExpect("-1", "0 -1");
    program.runAndExpect("1", "-2 1");
    program.runAndExpect("42", "-43 42");
}

// FIPS 180-1 SHA1 Ch constants: ~0xEFCDAB89 == 0x10325476
TEST(Compiler, bitwiseNotSha1Constant) {
    SourceProgram program{R"prg(
        int main() {
            unsigned int b;
            b = 4023233417u;
            printf("%u", ~b);
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("", "271733878");
}

// SHA1 Ch(b,c,d) = (b & c) | ((~b) & d) with classic IV
// b=0xEFCDAB89, c=0x98BADCFE, d=0x10325476 -> 0x98BADCFE
TEST(Compiler, bitwiseNotSha1Ch) {
    SourceProgram program{R"prg(
        int main() {
            unsigned int b;
            unsigned int c;
            unsigned int d;
            unsigned int f;
            b = 4023233417u;
            c = 2562383102u;
            d = 271733878u;
            f = (b & c) | ((~b) & d);
            printf("%u", f);
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("", "2562383102");
}

} // namespace
