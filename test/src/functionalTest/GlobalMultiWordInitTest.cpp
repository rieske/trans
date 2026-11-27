#include "TestFixtures.h"

namespace {

TEST(Compiler, globalStructBraceInitializer) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        struct S {
            int a;
            int b;
        };
        struct S g = { 1, 2 };
        int main() {
            printf("%d %d", g.a, g.b);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 2");
}

TEST(Compiler, globalStructPartialZeroFill) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        struct S {
            int a;
            int b;
            int c;
        };
        struct S g = { 5 };
        int main() {
            printf("%d %d %d", g.a, g.b, g.c);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("5 0 0");
}

TEST(Compiler, globalStructDesignatedInitializer) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        struct S {
            int a;
            int b;
            int c;
        };
        struct S g = { .b = 42 };
        int main() {
            printf("%d %d %d", g.a, g.b, g.c);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0 42 0");
}

TEST(Compiler, globalNestedDesignatedInitializer) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        struct Flags {
            int recursive;
            int dense;
        };
        struct Prune {
            void *orderfile;
            int reverse;
            struct Flags flags;
        };
        struct Rev {
            int remerge_diff;
            struct Prune pruning;
            int limited;
        };
        struct Rev g = {
            .pruning.flags.recursive = 1
        };
        int main() {
            if (g.pruning.orderfile != 0) return 1;
            if (g.pruning.reverse != 0) return 2;
            if (g.pruning.flags.recursive != 1) return 3;
            if (g.pruning.flags.dense != 0) return 4;
            printf("ok");
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("ok");
}

TEST(Compiler, globalArrayBraceInitializer) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int a[3] = { 1, 2, 3 };
        int main() {
            printf("%d %d %d", a[0], a[1], a[2]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 2 3");
}

TEST(Compiler, globalFlatNestedStruct) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        struct Inner {
            int a;
            int b;
        };
        struct Outer {
            struct Inner in;
            int w;
        };
        struct Outer g = { 1, 2, 3 };
        int main() {
            printf("%d %d %d", g.in.a, g.in.b, g.w);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 2 3");
}

TEST(Compiler, fileScopeFloatLiteral) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        static float g = 2.5f;
        int main(void) {
            printf("%d", (int)g);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("2");
}

TEST(Compiler, fileScopeDoubleLiteral) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        static double g = 2.5;
        int main(void) {
            printf("%d", (int)g);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("2");
}

TEST(Compiler, fileScopeNegativeFloat) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        static float g = -2.5f;
        int main(void) {
            printf("%d", (int)g);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("-2");
}

// Fuzz: file-scope long double constants were emitted as zero (16-byte x87 init).
TEST(Compiler, fileScopeLongDoubleLiteral) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        static long double g = 3.0L;
        int main(void) {
            printf("%d", (int)g);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3");
}

TEST(Compiler, fileScopeLongDoubleNonInteger) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        static long double g = 8.5L;
        int main(void) {
            printf("%d", (int)g);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("8");
}

TEST(Compiler, fileScopeLongDoubleNegative) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        static long double g = -3.0L;
        int main(void) {
            printf("%d", (int)g);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("-3");
}

TEST(Compiler, fileScopeStructLongDoubleLiteral) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct S { long double x; };
        static struct S g = { 3.0L };
        int main(void) {
            printf("%d", (int)g.x);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3");
}

TEST(Compiler, fileScopeDoubleFromLongDouble) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        static double d = 1.0L;
        int main(void) {
            printf("%d", (int)d);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, fileScopeIntFromLongDouble) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        static int i = 1.0L;
        int main(void) {
            printf("%d", i);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, fileScopeIntFromNegativeLongDouble) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        static int i = -3.0L;
        int main(void) {
            printf("%d", i);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("-3");
}

TEST(Compiler, fileScopeFloatingFromWideIntegers) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        double gd = 18446744073709551516ull;
        float gf = 18446744073709551516ull;
        long double gl = 18446744073709551516ull;
        long double gx = 9007199254740993LL;
        double gi = -((__int128)1 << 100);
        double gu = (unsigned __int128)1 << 127;
        int main(void) {
            printf("%.0f %.0f %.0Lf %.0Lf %.0f %.0f", gd, gf, gl, gx, gi, gu);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("18446744073709551616 18446744073709551616 18446744073709551516 9007199254740993 "
            "-1267650600228229401496703205376 170141183460469231731687303715884105728");
}

TEST(Compiler, fileScopeWideIntegersFromFloating) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        unsigned long long gu = 1.8e19;
        __int128 gi = -1e20;
        unsigned __int128 gb = 2e38;
        __int128 gm = -0x1p127;
        int main(void) {
            __int128 i = gi;
            unsigned __int128 b = gb;
            __int128 m = gm;
            printf("%llu %lld %llu %llu %llu %lld %llu", gu, (long long)(i >> 64), (unsigned long long)i,
                    (unsigned long long)(b >> 64), (unsigned long long)b,
                    (long long)(m >> 64), (unsigned long long)m);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("18000000000000000000 -6 10680464442257309696 10842021724855044096 0 "
            "-9223372036854775808 0");
}

TEST(Compiler, fileScopeFloatFromLongDoubleRoundsOnce) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        float gf = 1.00000005960464477625798674L;
        int main(void) {
            printf("%.9g", gf);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1.00000012");
}

TEST(Compiler, staticComplexFromRealValues) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        void *memcpy(void *, const void *, unsigned long);
        _Complex double gz = 7;
        _Complex float gf = 2.5;
        _Complex long double gl = 18446744073709551516ull;
        struct S { int k; _Complex double z; } gs = { 1, -3 };
        union U { long l[4]; _Complex long double z; } gn = { .l = { 1, 2, 3, 4 }, .z = 5 };
        int main(void) {
            double z[2];
            float f[2];
            long double l[2];
            double s[2];
            long double n[2];
            memcpy(z, &gz, sizeof z);
            memcpy(f, &gf, sizeof f);
            memcpy(l, &gl, sizeof l);
            memcpy(s, &gs.z, sizeof s);
            memcpy(n, &gn.z, sizeof n);
            printf("%g %g %g %g %.0Lf %Lg %d %g %g %Lg %Lg",
                    z[0], z[1], f[0], f[1], l[0], l[1], gs.k, s[0], s[1], n[0], n[1]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("7 0 2.5 0 18446744073709551516 0 1 -3 0 5 0");
}

TEST(Compiler, functionScopeStaticLongDoubleLiteral) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        long double get(void) {
            static long double g = 5.0L;
            return g;
        }
        int main(void) {
            printf("%d", (int)get());
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("5");
}

} // namespace
