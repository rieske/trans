#include "TestFixtures.h"

namespace {

TEST(Compiler, fileScopeUnsignedLongMaxLiteral) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        unsigned long g = 18446744073709551615UL;
        int main(void) {
            printf("%d %d", g == 0, g == 18446744073709551615UL);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0 1");
}

TEST(Compiler, fileScopeUnsignedLongMaxHex) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        unsigned long g = 0xffffffffffffffffUL;
        int main(void) {
            printf("%d", g == 0xffffffffffffffffUL);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, fileScopeUnsignedLongHighBitOnly) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        unsigned long g = 0x8000000000000000UL;
        int main(void) {
            printf("%d %d %d", g == 0, g == ~0UL, g == 0x8000000000000000UL);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0 0 1");
}

TEST(Compiler, fileScopeUnsignedIntWrapsWideLiteral) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        unsigned int g = 18446744073709551615UL;
        int main(void) {
            printf("%d %d", g == 0xffffffffU, g == 0);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 0");
}

TEST(Compiler, fileScopeUnsignedLongBitwiseNotZero) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        unsigned long g = ~0UL;
        int main(void) {
            printf("%d", g == ~0UL);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, fileScopeUnsignedLongCastMinusOne) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        unsigned long g = (unsigned long)-1;
        int main(void) {
            printf("%d", g == (unsigned long)-1);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, fileScopeGitTimeMaxShape) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        typedef unsigned long timestamp_t;
        #define TIME_MAX 18446744073709551615UL
        static timestamp_t cutoff = TIME_MAX;
        int main(void) {
            printf("%d", cutoff == TIME_MAX);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, fileScopeStructUnsignedLongMax) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct S {
            unsigned long t;
            int n;
        };
        struct S s = { 18446744073709551615UL, 3 };
        int main(void) {
            printf("%d %d", s.t == 18446744073709551615UL, s.n);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 3");
}

TEST(Compiler, functionScopeStaticUnsignedLongMax) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        unsigned long get(void) {
            static unsigned long g = 18446744073709551615UL;
            return g;
        }
        int main(void) {
            printf("%d", get() == 18446744073709551615UL);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

} // namespace
