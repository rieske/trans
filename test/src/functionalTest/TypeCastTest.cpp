#include "TestFixtures.h"

namespace {

TEST(Compiler, castIntToPointerAndBack) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            int x;
            int* p;
            x = 42;
            p = (int*)x;
            printf("%d", (int)p);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("42");
}

TEST(Compiler, castPointerAndIndex) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            int a[2];
            int* p;
            a[0] = 1;
            a[1] = 2;
            p = &a[0];
            printf("%d", ((int*)p)[1]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("2");
}

TEST(Compiler, castArrayToPointer) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            int a[2];
            int* p;
            a[0] = 7;
            a[1] = 8;
            p = (int*)a;
            printf("%d %d", p[0], p[1]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("7 8");
}

TEST(Compiler, castMultiDimRowToPointer) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            int a[2][3];
            int* p;
            a[1][2] = 42;
            p = (int*)a[1];
            printf("%d", p[2]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("42");
}


TEST(Compiler, castConstantArrayBound) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            int a[(int)3];
            a[0] = 1;
            a[1] = 2;
            a[2] = 3;
            printf("%d %d", a[2], sizeof a);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3 12");
}

// Int-to-float conversion in assignment (SSE path).
TEST(Compiler, intToDoubleAssignment) {
    SourceProgram program{R"prg(#include <stdio.h>
        int main() {
            int n = 21;
            double d;
            d = n;
            d = d + d;
            printf("%d", (int)d);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("42");
}

// Cast double → int local (cvttsd2si path).
TEST(Compiler, castDoubleToIntLocal) {
    SourceProgram program{R"prg(#include <stdio.h>
        int main() {
            double d = 42.9;
            int n = (int)d;
            printf("%d", n);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("42");
}

// Cast to array type is error.
TEST(Compiler, castToArrayTypeIsError) {
    SourceProgram program{R"prg(
        int main() {
            int x;
            x = 1;
            (int[4])x;
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("cast to array");
}

TEST(Compiler, castToBoolConvertsNonzeroToOne) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            printf("%d %d %d", (int)(bool)2, (int)(bool)0, (int)(bool)1.5);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 0 1");
}

TEST(Compiler, castUnsignedIntToLongZeroExtends) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int scanf(const char *, ...);
        int main() {
            unsigned u;
            long y;
            u = 0;
            u = u - 1;
            y = (long)u;
            printf("%d", (int)(y == -1));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0");
}

TEST(Compiler, castSignedIntToLongSignExtends) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int scanf(const char *, ...);
        int main() {
            int i;
            long y;
            i = -1;
            y = (long)i;
            printf("%d", (int)y);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("-1");
}

TEST(Compiler, implicitSignedIntToLongSignExtends) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int a[2];
            long y;
            a[0] = -1;
            a[1] = 0;
            y = a[0];
            printf("%d", (int)y);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("-1");
}

TEST(Compiler, implicitUnsignedIntToLongZeroExtends) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            unsigned a[2];
            long y;
            a[0] = 0;
            a[0] = a[0] - 1;
            a[1] = 0;
            y = a[0];
            printf("%d", (int)(y == -1));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0");
}

TEST(Compiler, castSignedLongToInt128SignExtends) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            long v;
            __int128 x;
            unsigned long *p;
            v = -1;
            x = (__int128)v;
            p = (unsigned long *)&x;
            printf("%d %d", (int)p[0], (int)p[1]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("-1 -1");
}

TEST(Compiler, castUnsignedLongToInt128ZeroExtends) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            unsigned long u;
            __int128 x;
            unsigned long *p;
            u = 0;
            u = u - 1;
            x = (__int128)u;
            p = (unsigned long *)&x;
            printf("%d %d", (int)(p[0] == u), (int)p[1]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 0");
}

TEST(Compiler, implicitSignedLongToInt128SignExtends) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            long v;
            __int128 x;
            unsigned long *p;
            v = -1;
            x = v;
            p = (unsigned long *)&x;
            printf("%d %d", (int)p[0], (int)p[1]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("-1 -1");
}

TEST(Compiler, implicitUnsignedLongToInt128ZeroExtends) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            unsigned long u;
            __int128 x;
            unsigned long *p;
            u = 0;
            u = u - 1;
            x = u;
            p = (unsigned long *)&x;
            printf("%d %d", (int)(p[0] == u), (int)p[1]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 0");
}

TEST(Compiler, castSignedIntToInt128SignExtends) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int i;
            __int128 x;
            unsigned long *p;
            i = -1;
            x = (__int128)i;
            p = (unsigned long *)&x;
            printf("%d %d", (int)p[0], (int)p[1]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("-1 -1");
}

TEST(Compiler, unsignedInt128FromSignedLongKeepsSourceSign) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            long v;
            unsigned __int128 x;
            unsigned long *p;
            v = -1;
            x = (unsigned __int128)v;
            p = (unsigned long *)&x;
            printf("%d %d", (int)p[0], (int)p[1]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("-1 -1");
}

TEST(Compiler, int128RoundtripNarrowPreservesLowWord) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            long v;
            __int128 x;
            long y;
            v = -1;
            x = (__int128)v;
            y = (long)x;
            printf("%d", (int)y);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("-1");
}

TEST(Compiler, implicitInt128ToLongTakesLowWord) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            long v;
            __int128 x;
            long y;
            v = -1;
            x = (__int128)v;
            y = x;
            printf("%d", (int)y);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("-1");
}

TEST(Compiler, castFunctionDesignatorToFunctionPointer) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int seven(void) { return 7; }
        int main(void) {
            int (*p)(void);
            p = (int (*)(void))seven;
            printf("%d", p());
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("7");
}

TEST(Compiler, castFunctionDesignatorToVoidPointer) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int seven(void) { return 7; }
        int main(void) {
            printf("%d", (int)((void *)seven != 0));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, int128WidenOnReturnAndNarrowOnArg) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        __int128 widen(long v) {
            return v;
        }
        long narrow(__int128 x) {
            return x;
        }
        int main() {
            printf("%d", (int)narrow(widen(-1)));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("-1");
}

} // namespace
