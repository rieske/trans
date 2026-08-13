#include "TestFixtures.h"

namespace {

TEST(Compiler, longVariableArithmetic) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            long a;
            long b;
            a = 100;
            b = 23;
            printf("%d", a + b);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("123");
}

TEST(Compiler, unsignedVariableArithmetic) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            unsigned a;
            unsigned b;
            a = 10;
            b = 7;
            printf("%d", a - b);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3");
}

TEST(Compiler, shortVariableArithmetic) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            short a;
            short b;
            a = 4;
            b = 5;
            printf("%d", a * b);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("20");
}

TEST(Compiler, signedVariableArithmetic) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            signed a;
            a = -3;
            printf("%d", -a);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3");
}

TEST(Compiler, longFunctionParameterAndReturn) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        long add(long x, long y) {
            return x + y;
        }
        int main() {
            printf("%d", add(40, 2));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("42");
}

TEST(Compiler, unsignedPointer) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            unsigned v;
            unsigned* p;
            v = 99;
            p = &v;
            *p = 11;
            printf("%d", v);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("11");
}

TEST(Compiler, unsignedIntIsUnsigned) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            unsigned int a;
            a = 5;
            printf("%d", a + 1);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("6");
}

TEST(Compiler, sizeofLongAndShort) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            printf("%d %d %d", (int)sizeof(long), (int)sizeof(short), (int)sizeof(unsigned));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("8 2 4");
}

TEST(Compiler, multiWordUnsignedLongLocal) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            unsigned long x;
            x = 41;
            printf("%d", x + 1);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("42");
}

TEST(Compiler, sizeofLongUnsignedOrderIndependent) {
    // type_name combine must not drop long when keywords are reordered.
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            printf("%d %d", (int)sizeof(long unsigned), (int)sizeof(long unsigned int));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("8 8");
}

TEST(Compiler, wideUnsignedIntegerLiteralsAssemble) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            unsigned long hi;
            unsigned long all;
            hi = 0xff00000000000000ULL;
            all = 18446744073709551615UL;
            printf("%d %d", (int)(hi == 0xff00000000000000ULL),
                (int)(all == 18446744073709551615UL));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 1");
}

TEST(Compiler, integerLiteralSuffixSetsWidth) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            printf("%d %d %d %d",
                (int)sizeof 1,
                (int)sizeof 1U,
                (int)sizeof 1L,
                (int)sizeof 1ULL);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4 4 8 8");
}

// Fuzzer: unsigned relational ops used signed jg/jl, so ~1u compared as -2.
TEST(Compiler, unsignedCompareAfterComplement) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            unsigned a;
            unsigned b;
            a = 1;
            a = ~a;
            b = 0;
            printf("%d %d %d", a > b, a < b, a <= b);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 0 0");
}

TEST(Compiler, unsignedLongCompareAfterComplement) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            unsigned long a;
            a = ~0UL;
            printf("%d %d", a > 0UL, a < 2UL);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 0");
}

// Fuzzer: unsigned % / used cqo+idiv, so ~0UL % 10 became -1.
TEST(Compiler, unsignedLongModuloOfAllOnes) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            unsigned long a;
            a = ~0UL;
            printf("%d %d", (int)(a % 10UL), (int)(a / 10UL != 0UL));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("5 1");
}

// Fuzzer: notq/negq on unsigned 32 left high bits set, then divq used the 64-bit value.
TEST(Compiler, unsignedIntDivAfterComplement) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            unsigned a;
            a = 19u;
            a = ~a;
            a = a / 7u;
            printf("%d", (int)a);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("613566753");
}

TEST(Compiler, unsignedIntModAfterComplement) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            unsigned a;
            a = 19u;
            a = ~a;
            a = a % 7u;
            printf("%d", (int)a);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("5");
}

TEST(Compiler, unsignedCompareAfterNotOfUnsignedLong) {
    // Fuzzer: `unsigned v1 = ~x` with unsigned long x left high bits set,
    // then 64-bit unsigned cmp treated 0xfffffffffffffffe > 0xffffffff as true.
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            unsigned long x;
            unsigned v1;
            unsigned v0;
            x = 1;
            v1 = ~x;
            v0 = 0xffffffffU;
            printf("%d", v1 > v0);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0");
}

TEST(Compiler, unsignedIntDivAfterNarrowingFromUnsignedLong) {
    // Fuzzer: 10ul-240ul assigned to unsigned kept the high 32 bits, then
    // 64-bit DIV used 0xffffffffffffff1a / 240 instead of 0xffffff1a / 240.
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            unsigned v;
            unsigned long w;
            w = 240;
            v = 10ul - w;
            v = v / 240u;
            printf("%u", v);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("17895696");
}

TEST(Compiler, unsignedIntDivAfterNegate) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            unsigned a;
            a = 18u;
            a = -a;
            a = a / 18u;
            printf("%d", (int)a);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("238609293");
}

TEST(Compiler, unsignedIntComplementWidensToUnsignedLong) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            unsigned a;
            unsigned long b;
            a = 0u;
            a = ~a;
            b = a;
            printf("%d", (int)(b / 2UL));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("2147483647");
}

TEST(Compiler, signedIntDivAfterWrap) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int a;
            a = 2147483647;
            a = a + 1;
            a = a / 3;
            printf("%d", a);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("-715827882");
}

} // namespace
