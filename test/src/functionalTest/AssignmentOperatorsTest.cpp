#include "TestFixtures.h"

namespace {

TEST(Compiler, assign) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
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
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
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
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
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
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
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
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
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
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
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

TEST(Compiler, unsignedLongDivModAssign) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            unsigned long a;
            unsigned long b;
            a = 0x8000000000000000UL;
            a /= 2UL;
            b = 0x8000000000000000UL;
            b %= 3UL;
            printf("%lu %lu", a, b);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4611686018427387904 2");
}

TEST(Compiler, andAssign) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
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
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
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
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
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
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
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

TEST(Compiler, shiftAssignPromotesNarrowCount) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            long x;
            unsigned char c;
            x = 1;
            c = 3;
            x <<= c;
            printf("%d", (int)x);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("8");
}

TEST(Compiler, shiftRightAssign) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
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

TEST(Compiler, unsignedShiftRightAssignIsLogical) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            unsigned u;
            unsigned long ul;
            u = 0x80000000u;
            u >>= 1;
            ul = 0x8000000000000000UL;
            ul >>= 1;
            printf("%u %lu", u, ul);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1073741824 4611686018427387904");
}

TEST(Compiler, addAssignThroughPointer) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            int n;
            int* p;
            n = 5;
            p = &n;
            *p += 3;
            printf("%d", n);
            return 0;
        }
    )prg"};

    program.compile();
    program.runAndExpect("8");
}

TEST(Compiler, subAssignThroughPointer) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            int n;
            int* p;
            n = 5;
            p = &n;
            *p -= 2;
            printf("%d", n);
            return 0;
        }
    )prg"};

    program.compile();
    program.runAndExpect("3");
}

TEST(Compiler, mulAssignThroughPointer) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            int n;
            int* p;
            n = 5;
            p = &n;
            *p *= 2;
            printf("%d", n);
            return 0;
        }
    )prg"};

    program.compile();
    program.runAndExpect("10");
}

TEST(Compiler, divAssignThroughPointer) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            int n;
            int* p;
            n = 10;
            p = &n;
            *p /= 2;
            printf("%d", n);
            return 0;
        }
    )prg"};

    program.compile();
    program.runAndExpect("5");
}

TEST(Compiler, modAssignThroughPointer) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            int n;
            int* p;
            n = 10;
            p = &n;
            *p %= 3;
            printf("%d", n);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, andAssignThroughPointer) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            int n;
            int* p;
            n = 7;
            p = &n;
            *p &= 3;
            printf("%d", n);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3");
}

TEST(Compiler, orAssignThroughPointer) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            int n;
            int* p;
            n = 1;
            p = &n;
            *p |= 2;
            printf("%d", n);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3");
}

TEST(Compiler, xorAssignThroughPointer) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            int n;
            int* p;
            n = 7;
            p = &n;
            *p ^= 3;
            printf("%d", n);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4");
}

TEST(Compiler, shiftLeftAssignThroughPointer) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            int n;
            int* p;
            n = 1;
            p = &n;
            *p <<= 3;
            printf("%d", n);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("8");
}

TEST(Compiler, shiftRightAssignThroughPointer) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            int n;
            int* p;
            n = 8;
            p = &n;
            *p >>= 2;
            printf("%d", n);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("2");
}

TEST(Compiler, compoundAssignComputesInFloatingType) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int i = 10;
            int j = 7;
            int k = 5;
            char c = 10;
            unsigned char uc = 3;
            short s = 3;
            long l = 9;
            int m = 3;
            i *= 0.5;
            j /= 0.5;
            k -= 0.5;
            c *= 2.55;
            uc *= 1.5;
            s *= 1.5;
            l /= 0.5;
            m *= 1.5L;
            printf("%d %d %d %d %d %d %ld %d", i, j, k, c, uc, s, l, m);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("5 14 4 25 4 4 18 4");
}

TEST(Compiler, compoundAssignWithLongDoubleRightOperand) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        unsigned gu = 2000000000u;
        int main() {
            signed char c = -10;
            short s = -300;
            unsigned u = 3000000000u;
            unsigned u2 = 2000000000u;
            unsigned long ul = 10000000000000000000UL;
            c /= 2.0L;
            s /= 2.0L;
            u /= 2.0L;
            u2 *= 2.0L;
            ul /= 2.0L;
            gu *= 2.0L;
            printf("%d %d %u %u %lu %u", c, s, u, u2, ul, gu);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("-5 -150 1500000000 4000000000 5000000000000000000 4000000000");
}

TEST(Compiler, compoundAssignInt128WithFloatingRightOperand) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        static void show(__int128 v) { printf("%llx:%llx ", (unsigned long long)(v >> 64), (unsigned long long)v); }
        int main() {
            __int128 y = (__int128)1 << 70;
            __int128 x = 10;
            __int128 v = -10;
            unsigned __int128 u = (unsigned __int128)3 << 100;
            y /= 0.5;
            x *= -0.5;
            v *= -0.5L;
            u *= 0.5f;
            show(y);
            show(x);
            show(v);
            show((__int128)u);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("80:0 ffffffffffffffff:fffffffffffffffb 0:5 1800000000:0 ");
}

TEST(Compiler, compoundAssignInt128WithComplexRightOperand) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            _Complex double z = -0.5;
            __int128 big = (__int128)1 << 70;
            big *= z;
            printf("%llx:%llx ", (unsigned long long)(big >> 64), (unsigned long long)big);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("ffffffffffffffe0:0 ");
}

TEST(Compiler, compoundAssignMixesRealAndComplex) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        void *memcpy(void *, const void *, unsigned long);
        int main() {
            float pf[2] = { 1.0f, 2.0f };
            double pd[2] = { 3.0, 4.0 };
            float of[2];
            _Complex float zf;
            _Complex double z;
            double d = 2.0;
            memcpy(&zf, pf, sizeof zf);
            memcpy(&z, pd, sizeof z);
            zf += 0.5;
            d *= z;
            memcpy(of, &zf, sizeof of);
            printf("%g %g %g", of[0], of[1], d);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1.5 2 6");
}

TEST(Compiler, compoundAssignIntegerWithComplexRightOperand) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        void *memcpy(void *, const void *, unsigned long);
        unsigned gu = 2000000000u;
        char gc = -10;
        int main() {
            double pd[2] = { 2.0, 0.0 };
            long double pl[2] = { 2.0L, 0.0L };
            _Complex double z;
            _Complex long double zl;
            memcpy(&z, pd, sizeof z);
            memcpy(&zl, pl, sizeof zl);
            unsigned u = 3000000000u;
            unsigned u2 = 2000000000u;
            char c = -10;
            unsigned long ul = 10000000000000000000UL;
            unsigned long ul2 = 5000000000000000000UL;
            u /= z;
            u2 *= z;
            c /= z;
            ul /= z;
            ul2 *= z;
            gu *= z;
            gc /= zl;
            printf("%u %u %d %lu %lu %u %d", u, u2, c, ul, ul2, gu, gc);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1500000000 4000000000 -5 5000000000000000000 10000000000000000000 4000000000 -5");
}

TEST(Compiler, compoundAssignDividesInWiderType) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            unsigned char c = 200;
            unsigned char c2 = 200;
            unsigned u = 7;
            int i = 7;
            c /= 258;
            c2 %= 258;
            u /= 0x100000002LL;
            i %= 0x100000003LL;
            printf("%d %d %u %d", c, c2, u, i);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0 200 0 7");
}

TEST(Compiler, compoundAssignUsesUnsignedCommonType) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int i = -1;
            int j = -7;
            long l = -1;
            unsigned char uc = 200;
            char c = -10;
            i /= 2u;
            j %= 3U;
            l /= 2ul;
            uc /= -1;
            c /= 3U;
            printf("%d %d %ld %d %d", i, j, l, uc, c);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("2147483647 0 9223372036854775807 56 82");
}

TEST(Compiler, compoundAssignToBoolConvertsResult) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            _Bool a = 1;
            _Bool b = 1;
            _Bool c = 1;
            _Bool d = 1;
            _Bool e = 1;
            struct { _Bool bit : 1; _Bool half : 1; } s = { 1, 1 };
            a &= 2;
            b ^= 2;
            c -= 2;
            d /= 2;
            e -= 0.5;
            s.bit -= 2;
            s.half -= 0.5;
            printf("%d %d %d %d %d %d %d", a, b, c, d, e, s.bit, s.half);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0 1 1 0 1 1 1");
}

TEST(Compiler, compoundAssignThroughPointerElementAndBitField) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int a[2] = { 10, 7 };
            int *p = a;
            struct { unsigned x : 3; int y : 5; } s = { 6, -6 };
            *p *= 0.5;
            a[1] /= 0.5;
            s.x /= -2;
            s.y *= 1.5;
            printf("%d %d %d %d", a[0], a[1], s.x, s.y);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("5 14 5 -9");
}

TEST(Compiler, compoundAssignYieldsConvertedLeftValue) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int i = 10;
            unsigned char c = 200;
            int r = (i *= 0.5) + 1;
            printf("%d %d %d", i, r, c /= 258);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("5 6 0");
}

TEST(Compiler, narrowShiftAssignTruncatesResult) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            unsigned char c = 0x81;
            signed char sc = -128;
            short s = 0x4001;
            unsigned char d = 200;
            signed char e = -1;
            c <<= 1;
            sc >>= 1;
            s <<= 2;
            d >>= 9;
            e >>= 9;
            printf("%d %d %d %d %d", c, sc, s, d, e);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("2 -64 4 0 -1");
}

}

