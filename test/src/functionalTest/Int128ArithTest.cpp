#include "TestFixtures.h"

namespace {

const char* kWordHelpers = R"prg(
        void set_words(__int128 *x, unsigned long lo, unsigned long hi) {
            unsigned long *p;
            p = (unsigned long *)x;
            p[0] = lo;
            p[1] = hi;
        }
        void print_words(__int128 x) {
            unsigned long *p;
            p = (unsigned long *)&x;
            printf("%d %d", (int)p[0], (int)p[1]);
        }
)prg";

TEST(Compiler, int128AddNoCarry) {
    SourceProgram program{std::string("int printf(const char *, ...);") + kWordHelpers + R"prg(
        int main() {
            __int128 a;
            __int128 b;
            set_words(&a, 1, 0);
            set_words(&b, 2, 0);
            print_words(a + b);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3 0");
}

TEST(Compiler, int128AddCarryIntoHighWord) {
    SourceProgram program{std::string("int printf(const char *, ...);") + kWordHelpers + R"prg(
        int main() {
            __int128 a;
            __int128 one;
            a = -1;
            set_words(&one, 1, 0);
            print_words(a + one);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0 0");
}

TEST(Compiler, int128AddHighWords) {
    SourceProgram program{std::string("int printf(const char *, ...);") + kWordHelpers + R"prg(
        int main() {
            __int128 a;
            __int128 b;
            set_words(&a, 42, 1);
            set_words(&b, 1, 1);
            print_words(a + b);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("43 2");
}

TEST(Compiler, int128AddTwoNegatives) {
    SourceProgram program{std::string("int printf(const char *, ...);") + kWordHelpers + R"prg(
        int main() {
            __int128 a;
            a = -1;
            print_words(a + a);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("-2 -1");
}

TEST(Compiler, int128AddMixedLong) {
    SourceProgram program{std::string("int printf(const char *, ...);") + kWordHelpers + R"prg(
        int main() {
            __int128 a;
            long b;
            set_words(&a, 0, 1);
            b = 1;
            print_words(a + b);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 1");
}

TEST(Compiler, int128SubBorrowFromHighWord) {
    SourceProgram program{std::string("int printf(const char *, ...);") + kWordHelpers + R"prg(
        int main() {
            __int128 a;
            __int128 one;
            set_words(&a, 0, 0);
            set_words(&one, 1, 0);
            print_words(a - one);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("-1 -1");
}

TEST(Compiler, int128SubHighWord) {
    SourceProgram program{std::string("int printf(const char *, ...);") + kWordHelpers + R"prg(
        int main() {
            __int128 a;
            __int128 one;
            set_words(&a, 0, 1);
            set_words(&one, 1, 0);
            print_words(a - one);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("-1 0");
}

TEST(Compiler, int128BitwiseBothWords) {
    SourceProgram program{std::string("int printf(const char *, ...);") + kWordHelpers + R"prg(
        int main() {
            __int128 a;
            __int128 b;
            set_words(&a, 3, 12);
            set_words(&b, 5, 10);
            print_words(a & b);
            printf(" ");
            print_words(a | b);
            printf(" ");
            print_words(a ^ b);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 8 7 14 6 6");
}

TEST(Compiler, int128BitwiseNot) {
    SourceProgram program{std::string("int printf(const char *, ...);") + kWordHelpers + R"prg(
        int main() {
            __int128 z;
            __int128 n;
            set_words(&z, 0, 0);
            n = -1;
            print_words(~z);
            printf(" ");
            print_words(~n);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("-1 -1 0 0");
}

TEST(Compiler, int128Negate) {
    SourceProgram program{std::string("int printf(const char *, ...);") + kWordHelpers + R"prg(
        int main() {
            __int128 one;
            __int128 wide;
            set_words(&one, 1, 0);
            set_words(&wide, 1, 1);
            print_words(-one);
            printf(" ");
            print_words(-wide);
            printf(" ");
            print_words(-(-one));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("-1 -1 -1 -2 1 0");
}

TEST(Compiler, int128Equality) {
    SourceProgram program{std::string("int printf(const char *, ...);") + kWordHelpers + R"prg(
        int main() {
            __int128 a;
            __int128 b;
            __int128 c;
            set_words(&a, 42, 1);
            set_words(&b, 42, 1);
            set_words(&c, 42, 2);
            printf("%d %d %d %d",
                (int)(a == b), (int)(a != b), (int)(a == c), (int)(a != c));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 0 0 1");
}

TEST(Compiler, int128SignedCompare) {
    SourceProgram program{std::string("int printf(const char *, ...);") + kWordHelpers + R"prg(
        int main() {
            __int128 neg;
            __int128 z;
            __int128 one;
            __int128 wide;
            neg = -1;
            set_words(&z, 0, 0);
            set_words(&one, 1, 0);
            set_words(&wide, 0, 1);
            printf("%d %d %d %d %d %d",
                (int)(neg < z), (int)(neg < one), (int)(z < one),
                (int)(wide > one), (int)(neg > wide), (int)(neg <= neg));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 1 1 1 0 1");
}

TEST(Compiler, int128UnsignedCompare) {
    SourceProgram program{std::string("int printf(const char *, ...);") + kWordHelpers + R"prg(
        int main() {
            unsigned __int128 n;
            unsigned __int128 z;
            unsigned __int128 one;
            n = (unsigned __int128)-1;
            set_words((__int128 *)&z, 0, 0);
            set_words((__int128 *)&one, 1, 0);
            printf("%d %d %d",
                (int)(n > z), (int)(n > one), (int)(z < one));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 1 1");
}

TEST(Compiler, int128CompareLowWordAfterEqualHigh) {
    SourceProgram program{std::string("int printf(const char *, ...);") + kWordHelpers + R"prg(
        int main() {
            __int128 a;
            __int128 b;
            set_words(&a, 1, 1);
            set_words(&b, 2, 1);
            printf("%d %d", (int)(a < b), (int)(b > a));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 1");
}

TEST(Compiler, int128LogicalNot) {
    SourceProgram program{std::string("int printf(const char *, ...);") + kWordHelpers + R"prg(
        int main() {
            __int128 z;
            __int128 one;
            __int128 wide;
            set_words(&z, 0, 0);
            set_words(&one, 1, 0);
            set_words(&wide, 0, 1);
            printf("%d %d %d", (int)!z, (int)!one, (int)!wide);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 0 0");
}

TEST(Compiler, int128CompareToZeroLiteral) {
    SourceProgram program{std::string("int printf(const char *, ...);") + kWordHelpers + R"prg(
        int main() {
            __int128 z;
            __int128 wide;
            set_words(&z, 0, 0);
            set_words(&wide, 0, 1);
            printf("%d %d", (int)(z == 0), (int)(wide == 0));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 0");
}

TEST(Compiler, int128CompoundAddSub) {
    SourceProgram program{std::string("int printf(const char *, ...);") + kWordHelpers + R"prg(
        int main() {
            __int128 a;
            __int128 one;
            set_words(&a, 0, 0);
            a = a - 1;
            set_words(&one, 1, 0);
            a += one;
            print_words(a);
            printf(" ");
            a -= one;
            print_words(a);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0 0 -1 -1");
}

TEST(Compiler, int128CompoundAssignWidensIntLiteral) {
    SourceProgram program{std::string("int printf(const char *, ...);") + kWordHelpers + R"prg(
        int main() {
            __int128 a;
            set_words(&a, 0, 0);
            a -= 1;
            print_words(a);
            printf(" ");
            a += 1;
            print_words(a);
            printf(" ");
            a &= 1;
            print_words(a);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("-1 -1 0 0 0 0");
}

TEST(Compiler, int128ShiftLeft) {
    SourceProgram program{std::string("int printf(const char *, ...);") + kWordHelpers + R"prg(
        int main() {
            __int128 a;
            set_words(&a, 1, 0);
            print_words(a << 0);
            printf(" ");
            print_words(a << 1);
            printf(" ");
            set_words(&a, 1, 1);
            print_words(a << 1);
            printf(" ");
            set_words(&a, 1, 0);
            print_words(a << 64);
            printf(" ");
            print_words(a << 65);
            printf(" ");
            set_words(&a, 2, 0);
            print_words(a << 63);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 0 2 0 2 2 0 1 0 2 0 1");
}

TEST(Compiler, int128ShiftRightSigned) {
    SourceProgram program{std::string("int printf(const char *, ...);") + kWordHelpers + R"prg(
        int main() {
            __int128 a;
            set_words(&a, 2, 2);
            print_words(a >> 1);
            printf(" ");
            set_words(&a, 0, 1);
            print_words(a >> 64);
            printf(" ");
            a = -1;
            print_words(a >> 1);
            printf(" ");
            print_words(a >> 64);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 1 1 0 -1 -1 -1 -1");
}

TEST(Compiler, int128ShiftRightUnsigned) {
    SourceProgram program{std::string("int printf(const char *, ...);") + kWordHelpers + R"prg(
        int main() {
            unsigned __int128 n;
            unsigned __int128 r;
            unsigned long *p;
            n = (unsigned __int128)-1;
            r = n >> 1;
            p = (unsigned long *)&r;
            printf("%d %d ", (int)(p[0] == (unsigned long)-1), (int)((p[1] >> 63) == 0));
            r = n >> 64;
            print_words(*(__int128 *)&r);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 1 -1 0");
}

TEST(Compiler, int128CompoundShift) {
    SourceProgram program{std::string("int printf(const char *, ...);") + kWordHelpers + R"prg(
        int main() {
            __int128 a;
            set_words(&a, 1, 0);
            a <<= 64;
            print_words(a);
            printf(" ");
            a >>= 64;
            print_words(a);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0 1 1 0");
}

TEST(Compiler, int128ShiftByCharCount) {
    SourceProgram program{std::string("int printf(const char *, ...);") + kWordHelpers + R"prg(
        int main() {
            __int128 a;
            char c;
            set_words(&a, 1, 0);
            c = 64;
            print_words(a << c);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0 1");
}

TEST(Compiler, int128Mul) {
    SourceProgram program{std::string("int printf(const char *, ...);") + kWordHelpers + R"prg(
        int main() {
            __int128 a;
            __int128 b;
            set_words(&a, 3, 0);
            set_words(&b, 4, 0);
            print_words(a * b);
            printf(" ");
            set_words(&a, 0, 1);
            set_words(&b, 2, 0);
            print_words(a * b);
            printf(" ");
            a = -1;
            set_words(&b, 2, 0);
            print_words(a * b);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("12 0 0 2 -2 -1");
}

TEST(Compiler, int128DivMod) {
    SourceProgram program{std::string("int printf(const char *, ...);") + kWordHelpers + R"prg(
        int main() {
            __int128 a;
            __int128 b;
            set_words(&a, 12, 0);
            set_words(&b, 4, 0);
            print_words(a / b);
            printf(" ");
            print_words(a % b);
            printf(" ");
            set_words(&a, 0, 3);
            set_words(&b, 0, 1);
            print_words(a / b);
            printf(" ");
            a = -6;
            set_words(&b, 2, 0);
            print_words(a / b);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3 0 0 0 3 0 -3 -1");
}

TEST(Compiler, int128DivSignedVsUnsigned) {
    SourceProgram program{std::string("int printf(const char *, ...);") + kWordHelpers + R"prg(
        int main() {
            __int128 n;
            unsigned __int128 u;
            unsigned __int128 r;
            unsigned long *p;
            n = -1;
            u = (unsigned __int128)-1;
            r = u / 2;
            p = (unsigned long *)&r;
            printf("%d %d", (int)((n / 2) == 0), (int)((p[1] >> 62) == 1));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 1");
}

TEST(Compiler, int128CompoundMulDiv) {
    SourceProgram program{std::string("int printf(const char *, ...);") + kWordHelpers + R"prg(
        int main() {
            __int128 a;
            set_words(&a, 0, 1);
            a *= 2;
            print_words(a);
            printf(" ");
            a /= 2;
            print_words(a);
            printf(" ");
            set_words(&a, 5, 1);
            a %= a - 5;
            print_words(a);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0 2 0 1 5 0");
}

TEST(Compiler, int128LiteralDecimalPow2_64) {
    SourceProgram program{std::string("int printf(const char *, ...);") + kWordHelpers + R"prg(
        int main() {
            print_words(18446744073709551616);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0 1");
}

TEST(Compiler, int128LiteralHexPow2_64Plus42) {
    SourceProgram program{std::string("int printf(const char *, ...);") + kWordHelpers + R"prg(
        int main() {
            print_words(0x10000000000000000);
            printf(" ");
            print_words(0x1000000000000002a);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0 1 42 1");
}

TEST(Compiler, int128LiteralUnsignedMax) {
    SourceProgram program{std::string("int printf(const char *, ...);") + kWordHelpers + R"prg(
        int main() {
            print_words(0xffffffffffffffffffffffffffffffff);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("-1 -1");
}

TEST(Compiler, int128LiteralSizeofAndAdd) {
    SourceProgram program{std::string("int printf(const char *, ...);") + kWordHelpers + R"prg(
        int main() {
            printf("%d ", (int)sizeof 18446744073709551616);
            printf("%d ", (int)sizeof 18446744073709551616U);
            print_words(18446744073709551616 + 1);
            printf(" ");
            printf("%d", (int)(0x10000000000000000 == 18446744073709551616));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("16 16 1 1 1");
}

// Control: __int128 as the first struct member already works.
TEST(Compiler, int128FirstStructMemberLayout) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct S {
            __int128 a;
            int b;
        };
        int main(void) {
            struct S s;
            s.a = 1;
            s.b = 2;
            printf("%d %d %d", (int)sizeof(struct S), (int)s.b, (int)(long)s.a);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("32 2 1");
}

// Keyword __int128 after another member must parse (GCC/SysV layout: pad to 16, size 32).
TEST(Compiler, int128NonFirstStructMemberLayout) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct S {
            int a;
            __int128 b;
        };
        int main(void) {
            struct S s;
            s.a = 3;
            s.b = 4;
            printf("%d %d %d %d", (int)sizeof(struct S), (int)__builtin_offsetof(struct S, b),
                    s.a, (int)(long)s.b);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("32 16 3 4");
}

TEST(Compiler, int128AfterCharStructMemberLayout) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct S {
            char c;
            __int128 x;
        };
        int main(void) {
            struct S s;
            s.c = 2;
            s.x = 40;
            printf("%d %d %d", (int)sizeof(struct S), (int)__builtin_offsetof(struct S, x),
                    (int)(s.c + (long)s.x));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("32 16 42");
}

// Two __int128 members: second keyword must also parse.
TEST(Compiler, int128TwoMembersInStruct) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct S {
            __int128 a;
            __int128 b;
        };
        int main(void) {
            struct S s;
            s.a = 1;
            s.b = 2;
            printf("%d %d", (int)sizeof(struct S), (int)(long)(s.a + s.b));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("32 3");
}

TEST(Compiler, int128NonFirstUnionMemberLayout) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        union U {
            int a;
            __int128 b;
        };
        int main(void) {
            union U u;
            u.b = 7;
            printf("%d %d", (int)sizeof(union U), (int)(long)u.b);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("16 7");
}

// Typedef form already works as a regression lock for the intended layout.
TEST(Compiler, int128NonFirstMemberViaTypedef) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        typedef __int128 i128;
        struct S {
            int a;
            i128 b;
        };
        int main(void) {
            struct S s;
            s.a = 3;
            s.b = 4;
            printf("%d %d %d %d", (int)sizeof(struct S), (int)__builtin_offsetof(struct S, b),
                    s.a, (int)(long)s.b);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("32 16 3 4");
}

TEST(Compiler, int128ConvertsToFloatingTypes) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            __int128 big = (__int128)1 << 70;
            __int128 neg = -big;
            unsigned __int128 top = (unsigned __int128)1 << 127;
            float f = big;
            double d = neg;
            long double ld = big;
            double ud = top;
            long double uld = top;
            printf("%.0f %.0f %.0Lf %.6e %.6Le", f, d, ld, ud, uld);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect(
            "1180591620717411303424 -1180591620717411303424 1180591620717411303424 1.701412e+38 1.701412e+38");
}

TEST(Compiler, int128ConvertsToAndFromComplex) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        void *memcpy(void *, const void *, unsigned long);
        static void show(__int128 v) { printf("%llx:%llx ", (unsigned long long)(v >> 64), (unsigned long long)v); }
        int main() {
            double pm[2] = { -10.0, 3.0 };
            long double pl[2] = { 0x1p70L, 1.0L };
            _Complex double zm;
            _Complex long double zl;
            memcpy(&zm, pm, sizeof zm);
            memcpy(&zl, pl, sizeof zl);
            __int128 a = zm;
            __int128 b = zl;
            unsigned __int128 u = zl;
            _Complex double c = (__int128)1 << 70;
            _Complex long double d = -((__int128)1 << 70);
            double oc[2];
            long double od[2];
            memcpy(oc, &c, sizeof oc);
            memcpy(od, &d, sizeof od);
            show(a);
            show(b);
            show((__int128)u);
            printf("%.0f %.0f %.0Lf %.0Lf", oc[0], oc[1], od[0], od[1]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("ffffffffffffffff:fffffffffffffff6 40:0 40:0 "
            "1180591620717411303424 0 -1180591620717411303424 0");
}

TEST(Compiler, int128ImplicitArgumentConversionKeepsOtherArguments) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        void f(int a, double d) { printf("%d %g ", a, d); }
        void g(int a, __int128 v) { printf("%d %lld ", a, (long long)v); }
        int main() {
            __int128 x = 5;
            f(7, x);
            g(7, 9.0);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("7 5 7 9 ");
}

TEST(Compiler, int128ConvertsFromFloatingTypes) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        static void show(__int128 v) { printf("%llx:%llx ", (unsigned long long)(v >> 64), (unsigned long long)v); }
        int main() {
            __int128 a = 1e21;
            __int128 b = -10.0;
            __int128 c = -1.5f;
            __int128 e = 0x1p100L;
            unsigned __int128 u = 0x1p127;
            unsigned __int128 uf = 0x1p100f;
            show(a);
            show(b);
            show(c);
            show(e);
            show((__int128)u);
            show((__int128)uf);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("36:35c9adc5dea00000 ffffffffffffffff:fffffffffffffff6 ffffffffffffffff:ffffffffffffffff "
            "1000000000:0 8000000000000000:0 1000000000:0 ");
}

TEST(Compiler, int128GlobalOperands) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        __int128 gz = 0;
        __int128 ga = 1;
        __int128 gb = 2;
        unsigned __int128 gu = 0xffffffffffffffffull;
        unsigned __int128 gv = 1;
        unsigned __int128 gm = (unsigned __int128)0xffffffffffffffffull << 64;
        unsigned __int128 gs;
        int main(void) {
            int taken = 0;
            if (gz) {
                taken = 1;
            }
            unsigned __int128 sum = gu + gv;
            unsigned __int128 diff = gv - gu;
            gs = gu + gv;
            printf("%d %d %llx %llx %llx %llx %llx %llx %llx", taken, ga < gb,
                    (unsigned long long)(sum >> 64), (unsigned long long)sum,
                    (unsigned long long)(diff >> 64), (unsigned long long)diff,
                    (unsigned long long)(gm >> 64),
                    (unsigned long long)(gs >> 64), (unsigned long long)gs);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0 1 1 0 ffffffffffffffff 2 ffffffffffffffff 1 0");
}

TEST(Compiler, int128GlobalArgumentsAndReturns) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct P { long a; long b; };
        unsigned __int128 gw = ((unsigned __int128)3 << 64) | 5;
        struct P gp = { 7, 9 };
        unsigned long long high(unsigned __int128 v) { return (unsigned long long)(v >> 64); }
        unsigned long long low(unsigned __int128 v) { return (unsigned long long)v; }
        unsigned __int128 get(void) { return gw; }
        long sum(struct P p) { return p.a + p.b; }
        long last(long a, long b, long c, long d, long e, long f, unsigned __int128 v) {
            return a + f + (long)(v >> 64);
        }
        int main(void) {
            unsigned __int128 r = get();
            printf("%llu %llu %llu %llu %ld %ld", high(gw), low(gw), (unsigned long long)(r >> 64),
                    (unsigned long long)r, sum(gp), last(1, 2, 3, 4, 5, 6, gw));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3 5 3 5 16 10");
}

} // namespace
